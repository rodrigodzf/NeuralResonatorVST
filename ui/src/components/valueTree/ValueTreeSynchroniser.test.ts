import { readFileSync } from "fs";
import { join } from "path";
import { TEST_ASSETS_PATH } from "../../../test/utils";
import { InputStream } from "./InputStream";
import { ValueTree } from "./ValueTree";
import { applyChange } from "./ValueTreeSynchroniser";

const getTree = () => {
  const file = readFileSync(
    join(TEST_ASSETS_PATH, "valueTree", "valueTreeFull.bin")
  );

  const tree = ValueTree.readFromStream(new InputStream(new Uint8Array(file)));
  return tree;
};

const readChange = (name: string) => {
  const data = new Uint8Array(
    readFileSync(join(TEST_ASSETS_PATH, "syncMessages", `${name}.bin`))
  );
  return data;
};

describe("applyChange", () => {
  test("change property", () => {
    const tree = getTree();
    applyChange(tree, readChange("setStringProperty"));
    expect(tree.properties.get("stringProperty")).toEqual("Test 2");
  });

  test("change property - int (was failing in real app)", () => {
    const tree = getTree();
    applyChange(tree, readChange("setIntPropertyTo1503666456"));
    expect(tree.properties.get("intProperty")).toEqual(1503666456);
  });

  test("remove property", () => {
    const tree = getTree();
    applyChange(tree, readChange("removeStringProperty"));
    expect(tree.properties.has("stringProperty")).toBeFalsy();
  });

  test("add child at start", () => {
    const tree = getTree();
    applyChange(tree, readChange("addChildAtStart"));
    expect(tree.children).toHaveLength(3);
    expect(tree.children[0].type).toEqual("TestChild3");
    expect(tree.children[1].type).toEqual("TestChild1");
    expect(tree.children[2].type).toEqual("TestChild2");
  });

  test("add child in middle", () => {
    const tree = getTree();
    applyChange(tree, readChange("addChildInMiddle"));
    expect(tree.children).toHaveLength(3);
    expect(tree.children[0].type).toEqual("TestChild1");
    expect(tree.children[1].type).toEqual("TestChild3");
    expect(tree.children[2].type).toEqual("TestChild2");
  });

  test("add child at end", () => {
    const tree = getTree();
    applyChange(tree, readChange("addChildAtEnd"));
    expect(tree.children).toHaveLength(3);
    expect(tree.children[0].type).toEqual("TestChild1");
    expect(tree.children[1].type).toEqual("TestChild2");
    expect(tree.children[2].type).toEqual("TestChild3");
  });

  test("remove child", () => {
    const tree = getTree();
    applyChange(tree, readChange("removeChild2"));
    expect(tree.children).toHaveLength(1);
    expect(tree.children[0].type).toEqual("TestChild1");
  });

  test("move child", () => {
    const tree = getTree();
    applyChange(tree, readChange("moveChild"));
    expect(tree.children).toHaveLength(2);
    expect(tree.children[0].type).toEqual("TestChild2");
    expect(tree.children[1].type).toEqual("TestChild1");
  });
});
