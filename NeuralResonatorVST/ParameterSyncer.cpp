#include <random>
#include <time.h>
#include "HelperFunctions.h"
#include "ParameterSyncer.h"
#include <kac_core.hpp>
ParameterSyncer::ParameterSyncer(juce::AudioProcessorValueTreeState& vtsRef)
    : juce::ValueTreeSynchroniser(vtsRef.state)
    , mVTSRef(vtsRef)
{
}

ParameterSyncer::~ParameterSyncer() {}

ParameterSyncerIf* ParameterSyncer::getParameterSyncerIfPtr()
{
    return this;
}

void ParameterSyncer::setServerThreadIf(ServerThreadIf* serverThreadIfPtr)
{
    if (serverThreadIfPtr != nullptr)
    {
        mServerThreadIfPtr = serverThreadIfPtr;
    }
}

void ParameterSyncer::stateChanged(
    const void* encodedChange,
    size_t encodedChangeSize
)
{
    // encode as a base64 string
    auto changesAsBase64 =
        juce::Base64::toBase64(encodedChange, encodedChangeSize);

    // JLOG("ParameterSyncer::stateChanged: " +
    //  mVTSRef.state.toXmlString());

    JLOG("ParameterSyncer::stateChanged: sending to server");
    mServerThreadIfPtr->sendMessage(changesAsBase64);
}

void ParameterSyncer::receivedParameterChange(const juce::var& parameter)
{
    juce::MessageManager::callAsync(
        [this, parameter]()
        {
            // update the state tree
            auto parameterID = parameter["id"].toString();
            auto newValue = parameter["value"];

            // JLOG(
            // "ParameterSyncer::receivedParameterChange: " + parameterID +
            // " " + newValue.toString());

            mVTSRef.getParameterAsValue(parameterID) = newValue;
            // mVTSRef.state.setPropertyExcludingListener(this, parameterID,
            // newValue, nullptr);
        }
    );
}

void ParameterSyncer::receivedNewShape(const juce::var& shape)
{
    juce::MessageManager::callAsync(
        [this, shape]()
        {
            auto polygonTree =
                mVTSRef.state.getOrCreateChildWithName("polygon", nullptr);
            auto polygon = kac_core::geometry::normalisePolygon(
                kac_core::geometry::generateConvexPolygon(float(shape["value"]
                ))
                // kac_core::geometry::generateIrregularStar(float(shape["value"])
                // kac_core::geometry::generatePolygon(float(shape["value"])
            );

            juce::Array<juce::var> vertices;

            for (int i = 0; i < polygon.size(); ++i)
            {
                vertices.add(juce::var((polygon[i].x * 2.0f) - 1.));
                vertices.add(juce::var((polygon[i].y * 2.0f) - 1.));
            }

            polygonTree.setProperty("value", vertices, nullptr);
        }
    );
}

void ParameterSyncer::receivedShapeUpdate(const juce::var& shape)
{
    juce::MessageManager::callAsync(
        [this, shape]()
        {
            DBG("ParameterSyncer::receivedShapeUpdate");
            auto polygonTree = mVTSRef.state.getChildWithName("polygon");
            auto vertices = shape["value"];
            // change list to T::Polygon
            kac_core::types::Polygon P;
            int N = vertices.size() / 2;
            for (int i = 0; i < vertices.size(); i += 2)
            {
                P.push_back(kac_core::types::Point(
                    double(vertices[i]),
                    double(vertices[i + 1])
                ));
            }
            // 2 opt loop
            std::vector<std::pair<int, int>> indices;
            std::string intersection_type = "";
            bool intersections = true;
            while (intersections)
            {
            Search_loop:
                for (int i = 0; i < N - 2; i++)
                {
                    for (int j = i + 1; j < N; j++)
                    {
                        // collect indices of lines which should be crossed
                        intersection_type =
                            kac_core::geometry::lineIntersection(
                                T::Line(P[i], P[i + 1]),
                                T::Line(P[j], P[(j + 1) % N])
                            )
                                .first;
                        if (intersection_type == "none" ||
                            intersection_type == "vertex")
                        {
                            continue;
                        }
                        else if (intersection_type == "intersect")
                        {
                            indices.push_back(std::make_pair(i + 1, j + 1));
                        }
                        else if (intersection_type == "adjacent")
                        {
                            indices.push_back(std::make_pair(i, j));
                        }
                        else if (intersection_type == "colinear")
                        {
                            int min_i = 0 ? P[i].x < P[i + 1].x : 1;
                            int max_j = 0 ? P[j].x > P[j + 1].x : 1;
                            std::reverse(
                                P.begin() + i + min_i,
                                P.begin() + j + max_j
                            );
                            // restart loop
                            indices.clear();
                            goto Search_loop;
                        }
                    }
                }
                if (indices.size() > 0)
                {
                    // randomly swap one pair
                    std::pair<int, int> swap = indices
                        [abs(uniform_distribution(random_engine)) %
                         indices.size()];
                    std::reverse(
                        P.begin() + swap.first,
                        P.begin() + swap.second
                    );
                    // restart loop
                    indices.clear();
                    goto Search_loop;
                }
                else
                {
                    // close loop
                    intersections = false;
                }
            }
            // convert back to list
            for (int n = 0; n < N; n++)
            {
                vertices[2 * n] = juce::var(P[n].x);
                vertices[(2 * n) + 1] = juce::var(P[n].y);
            }
            // update
            polygonTree.setProperty("value", vertices, nullptr);
        }
    );
}

void ParameterSyncer::onOpen()
{
    // send the full state tree to the server
    // TODO: probably we will better wait for the cliet to request the
    // full state tree
    sendFullSyncCallback();
}

void ParameterSyncer::onClose()
{
    // do nothing
}