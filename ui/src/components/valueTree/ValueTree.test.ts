import { readFileSync } from "fs";
import { join } from "path";
import { TEST_ASSETS_PATH } from "../../../test/utils";
import { InputStream } from "./InputStream";
import { ValueTree } from "./ValueTree";

describe("readFromStream", () => {
  test("read tree successfully", () => {
    const file = readFileSync(
      join(TEST_ASSETS_PATH, "valueTree", "valueTreeFull.bin")
    );
    const tree = ValueTree.readFromStream(
      new InputStream(new Uint8Array(file))
    );

    expect(tree.type).toEqual("TestTree");
    expect(tree.parent).toEqual(undefined);
    expect(tree.isValid()).toBeTruthy();

    expect(tree.properties.size).toEqual(6);
    expect(tree.properties.get("stringProperty")).toEqual("Test");
    expect(tree.properties.get("intProperty")).toEqual(1234);
    expect(tree.properties.get("int64Property")!.toString()).toEqual(
      "9223372036854775800"
    );
    expect(tree.properties.get("doubleProperty")).toEqual(0.1234567);
    expect(tree.properties.get("boolProperty")).toEqual(true);
    expect(tree.properties.get("arrayProperty")).toEqual(["Test", 1234, true]);

    expect(tree.children).toHaveLength(2);

    expect(tree.children[0].type).toEqual("TestChild1");
    expect(tree.children[0].parent).toEqual(tree);
    expect(tree.children[0].isValid()).toBeTruthy();

    expect(tree.children[0].properties.size).toEqual(1);
    expect(tree.children[0].properties.get("stringProperty")).toEqual(
      "TestChildProperty"
    );

    expect(tree.children[0].children).toHaveLength(1);
    expect(tree.children[0].children[0].type).toEqual("TestNestedChild1");
    expect(tree.children[0].children[0].parent).toEqual(tree.children[0]);
    expect(tree.children[0].children[0].isValid()).toBeTruthy();

    expect(tree.children[0].children[0].properties.size).toEqual(1);
    expect(
      tree.children[0].children[0].properties.get("stringProperty")
    ).toEqual("TestNestedChildProperty");

    expect(tree.children[1].type).toEqual("TestChild2");
    expect(tree.children[1].parent).toEqual(tree);
    expect(tree.children[1].isValid()).toBeTruthy();

    expect(tree.children[1].properties.size).toEqual(0);
    expect(tree.children[1].children).toHaveLength(0);
  });
});
