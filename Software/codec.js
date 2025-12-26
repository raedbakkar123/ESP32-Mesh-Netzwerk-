// ChirpStack v4
// Erwartet 6 Bytes: [nodeId LE (4)] [temp_x10 int16 LE (2)]
function decodeUplink(input) {
  const bytes = input.bytes;
  const fPort = input.fPort;

  if (!bytes || bytes.length !== 6) {
    return { errors: ["unsupported length " + (bytes ? bytes.length : 0)] };
  }

  // nodeId (uint32, little-endian)
  let nodeId = (bytes[0]) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
  nodeId >>>= 0;

  // temperature * 10 (int16, little-endian)
  let t10 = (bytes[4]) | (bytes[5] << 8);
  if (t10 & 0x8000) t10 -= 0x10000;  // signed
  const temperatureC = t10 / 10.0;

  if (fPort < 1 || fPort > 223) {
    return { errors: ["unsupported port " + fPort] };
  }

  return { data: { port: fPort, nodeId, temperatureC } };
}
