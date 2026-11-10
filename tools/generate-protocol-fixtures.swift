import Foundation
import BitFoundation
guard CommandLine.arguments.count == 2 else { fatalError("Pass the output JSON file path") }
let sender=Data([0,1,2,3,4,5,6,7])
let recipient=Data([8,9,10,11,12,13,14,15])
let signature=Data((0..<64).map{UInt8($0)})
var rows:[[String:String]]=[]
for (name,version,directed,signed,routed,padded) in [
 ("v1-public",UInt8(1),false,false,false,false),
 ("v1-signed",UInt8(1),false,true,false,false),
 ("v1-directed",UInt8(1),true,true,false,false),
 ("v2-route",UInt8(2),true,true,true,false),
 ("v1-padded",UInt8(1),false,true,false,true)] {
 let packet=BitchatPacket(type:2,senderID:sender,recipientID:directed ? recipient:nil,timestamp:0x0102030405060708,payload:Data("hello".utf8),signature:signed ? signature:nil,ttl:7,version:version,route:routed ? [sender,recipient]:nil)
 let bytes=packet.toBinaryData(padding:padded)!
 rows.append(["name":name,"hex":bytes.map{String(format:"%02x",$0)}.joined()])
}
let data=try JSONSerialization.data(withJSONObject:rows,options:[.prettyPrinted,.sortedKeys])
try data.write(to:URL(fileURLWithPath:CommandLine.arguments[1]))
print("Generated \(rows.count) vectors with the actual pinned upstream serializer")
