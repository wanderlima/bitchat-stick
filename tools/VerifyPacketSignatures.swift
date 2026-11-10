import Foundation
import CryptoKit
func unhex(_ text:Substring)->Data {
    var result=Data();var offset=text.startIndex
    while offset<text.endIndex {let next=text.index(offset,offsetBy:2);result.append(UInt8(text[offset..<next],radix:16)!);offset=next}
    return result
}
let source=try String(contentsOfFile:CommandLine.arguments[1],encoding:.utf8)
var checked=0
for line in source.split(separator:"\n") {
    let parts=line.split(separator:" ");precondition(parts.count==4)
    let key=try Curve25519.Signing.PublicKey(rawRepresentation:unhex(parts[2]))
    precondition(key.isValidSignature(unhex(parts[3]),for:unhex(parts[1])),"Local signature rejected by CryptoKit")
    checked+=1
}
precondition(checked==3)
print("CryptoKit independently accepted all three locally generated packet signatures")
