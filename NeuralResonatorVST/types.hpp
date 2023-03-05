/*
🙇‍♀️🙇🙇‍♂️
*/

#pragma once

// core
#include <math.h>
#include <vector>

namespace kac_core::types {

	typedef struct Point {
		/*
		A point on the Euclidian plane.
		*/

		// vars
		double x = 0.0;
		double y = 0.0;
		double r() { return sqrt(pow(x, 2) + pow(y, 2)); }
		double theta() { return atan2(y, x); }

		// constructors
		Point() {};
		Point(double x, double y): x(x), y(y) {};

		// methods
		void updatePol(double r, double theta) {
			/*
			Update the point using polar coordinates.
			*/

			x = r * cos(theta);
			y = r * sin(theta);
		}
	} Point;

	typedef struct Line {
		/*
		A straight line from point a to point b.
		*/

		// vars
		Point a;
		Point b;

		// constructors
		Line() {};
		Line(Point a, Point b): a(a), b(b) {};
	} Line;

	// A polygon defined on the euclidian plane.
	typedef std::vector<Point> Polygon;

	typedef struct Circle {
		/*
		A circle defined on the euclidian plane.
		*/

		// vars
		double r = 1.0;	   // radius
		Point origin;	   // center

		// constructors
		Circle() {};
		Circle(double r): r(r) {};
		Circle(Point origin): origin(origin) {};
		Circle(double r, Point origin): r(r), origin(origin) {};
	} Circle;

	typedef struct Ellipse {
	} Ellipse;

	// Matrices
	typedef std::vector<double> Matrix_1D;
	typedef std::vector<std::vector<double>> Matrix_2D;
	typedef std::vector<std::vector<short>> BooleanImage;

}