import Foundation
import CryptoKit
import BitFoundation
let key=try Curve25519.Signing.PrivateKey(rawRepresentation:Data((0..<32).map{UInt8($0)}))
func hex(_ d:Data)->String{d.map{String(format:"%02x",$0)}.joined()}
let sender=Data([0,1,2,3,4,5,6,7])
for (name,text,version,route) in [("short","hello",UInt8(1),false),("compressed",String(repeating:"hello ",count:100),UInt8(1),false),("route","hello",UInt8(2),true)] {
 let packet=BitchatPacket(type:2,senderID:sender,recipientID:route ? Data(repeating:9,count:8):nil,timestamp:0x0102030405060708,payload:Data(text.utf8),signature:nil,ttl:7,version:version,route:route ? [sender,Data(repeating:9,count:8)]:nil)
 let preimage=packet.toBinaryDataForSigning()!
 var signed=packet;signed.signature=try key.signature(for:preimage)
 print(name+" "+hex(signed.toBinaryData(padding:false)!)+" "+hex(preimage)+" "+hex(key.publicKey.rawRepresentation))
}
