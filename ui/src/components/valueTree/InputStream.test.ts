import { InputStream, JuceVariant } from "./InputStream";
import { TextEncoder } from "util";

describe("readString", () => {
  test("read multiple strings", () => {
    const encoder = new TextEncoder();
    const stream = new InputStream(
      encoder.encode(
        "1test1" + String.fromCharCode(0) + "2test2" + String.fromCharCode(0)
      )
    );

    expect(stream.readString()).toEqual("1test1");
    expect(stream.readString()).toEqual("2test2");
    expect(stream.readString()).toEqual("");
  });
});

describe("readInt", () => {
  test("1 byte", () => {
    const stream = new InputStream(Uint8Array.from([0x7b]));
    expect(stream.readInt()).toEqual(123);
  });

  test("2 bytes", () => {
    const stream = new InputStream(Uint8Array.from([0xd2, 0x04]));
    expect(stream.readInt()).toEqual(1234);
  });

  test("3 bytes", () => {
    const stream = new InputStream(Uint8Array.from([0x87, 0xd6, 0x12]));
    expect(stream.readInt()).toEqual(1234567);
  });

  test("4 bytes", () => {
    const stream = new InputStream(Uint8Array.from([0x15, 0xcd, 0x5b, 0x07]));
    expect(stream.readInt()).toEqual(123456789);
  });
});

describe("readInt64", () => {
  test("8 bytes", () => {
    const stream = new InputStream(
      Uint8Array.from([0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f])
    );
    expect(stream.readInt64().toString()).toEqual("9223372036854775800");
  });
});

describe("readDouble", () => {
  test("8 bytes", () => {
    const stream = new InputStream(
      Uint8Array.from([0x72, 0xda, 0xf8, 0xb8, 0xdb, 0x9a, 0xbf, 0x3f])
    );
    expect(stream.readDouble()).toEqual(0.1234567);
  });
});

// Using compressedInt values written to a file in JUCE
describe("readCompressedInt", () => {
  test("1 byte", () => {
    const compressed = [0x01, 0x7b];
    const stream = new InputStream(Uint8Array.from(compressed));
    expect(stream.readCompressedInt()).toEqual(123);
  });

  test("2 bytes", () => {
    const compressed = [0x02, 0xd2, 0x04];
    const stream = new InputStream(Uint8Array.from(compressed));
    expect(stream.readCompressedInt()).toEqual(1234);
  });

  test("3 bytes", () => {
    const compressed = [0x03, 0x87, 0xd6, 0x12];
    const stream = new InputStream(Uint8Array.from(compressed));
    expect(stream.readCompressedInt()).toEqual(1234567);
  });

  test("4 bytes", () => {
    const compressed = [0x04, 0x15, 0xcd, 0x5b, 0x07];
    const stream = new InputStream(Uint8Array.from(compressed));
    expect(stream.readCompressedInt()).toEqual(123456789);
  });
});

// Using var values written to a file in JUCE
describe("readVar", () => {
  test("int", () => {
    const stream = new InputStream(
      Uint8Array.from([0x01, 0x05, 0x01, 0xd2, 0x04, 0x00, 0x00])
    );

    const value = stream.readVar();
    expect(typeof value).toEqual("number");
    expect(value).toEqual(1234);
  });

  test("int64", () => {
    const stream = new InputStream(
      // prettier-ignore
      Uint8Array.from([
        0x01, 0x09, 0x06, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
      ])
    );

    const value = stream.readVar();
    expect(typeof value).toEqual("bigint");
    expect(value!.toString()).toEqual("9223372036854775800");
  });

  test("bool true", () => {
    const stream = new InputStream(Uint8Array.from([0x01, 0x01, 0x02]));

    const value = stream.readVar();
    expect(typeof value).toEqual("boolean");
    expect(value).toEqual(true);
  });

  test("bool false", () => {
    const stream = new InputStream(Uint8Array.from([0x01, 0x01, 0x03]));

    const value = stream.readVar();
    expect(typeof value).toEqual("boolean");
    expect(value).toEqual(false);
  });

  test("string", () => {
    const stream = new InputStream(
      Uint8Array.from([0x01, 0x06, 0x05, 0x54, 0x65, 0x73, 0x74, 0x00])
    );

    const value = stream.readVar();
    expect(typeof value).toEqual("string");
    expect(value).toEqual("Test");
  });

  test("array", () => {
    const stream = new InputStream(
      // prettier-ignore
      Uint8Array.from([
        0x01, 0x15, 0x07, 0x01, 0x03, 0x01, 0x06, 0x05, 0x54, 0x65, 0x73, 0x74, 0x00, 0x01, 0x05, 0x01, 0xd2, 0x04, 0x00, 0x00, 0x01, 0x01, 0x02,
      ])
    );

    const value = stream.readVar() as JuceVariant[];
    expect(Array.isArray(value)).toBeTruthy();
    expect(typeof value[0]).toEqual("string");
    expect(value[0]).toEqual("Test");
    expect(typeof value[1]).toEqual("number");
    expect(value[1]).toEqual(1234);
    expect(typeof value[2]).toEqual("boolean");
    expect(value[2]).toEqual(true);
  });
});
