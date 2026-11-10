import AppKit
let source=URL(fileURLWithPath:CommandLine.arguments[1])
let target=URL(fileURLWithPath:CommandLine.arguments[2])
let files=try FileManager.default.contentsOfDirectory(at:source,includingPropertiesForKeys:nil).filter{$0.pathExtension=="png"}.sorted{$0.path<$1.path}
func save(_ image:NSImage,_ name:String) {
 let rep=NSBitmapImageRep(data:image.tiffRepresentation!)!
 try! rep.representation(using:.jpeg,properties:[.compressionFactor:0.98])!.write(to:target.appendingPathComponent(name))
}
for url in files { save(NSImage(contentsOf:url)!,url.deletingPathExtension().lastPathComponent+".jpg") }
let board=NSImage(size:NSSize(width:2240,height:1110));board.lockFocusFlipped(true)
NSColor(srgbRed:0.94,green:0.95,blue:0.93,alpha:1).setFill();NSBezierPath(rect:NSRect(x:0,y:0,width:2240,height:1110)).fill()
func text(_ text:String,_ x:CGFloat,_ y:CGFloat,_ size:CGFloat,_ bold:Bool=false) {
 (text as NSString).draw(at:NSPoint(x:x,y:y),withAttributes:[.font:NSFont(name:bold ? "Arial-BoldMT":"ArialMT",size:size)!, .foregroundColor:NSColor(srgbRed:0.08,green:0.14,blue:0.11,alpha:1)])
}
text("BITCHAT STICK",48,31,38,true)
text("FreeMono / cream #EBDCBA / 135 × 240 / simulated data",50,86,19)
let names=["07-splash","01-dashboard","02-nearby-peers","03-channels","04-channel-messages","05-infobot","06-settings","08-brightness","10-battery","09-about","11-diagnostics"]
let titles=["START","HOME","NEARBY","INBOX","MESSAGES","INFOBOT","SETTINGS","BRIGHTNESS","BATTERY","ABOUT","DIAGNOSTICS"]
for i in 0..<11 {
 let x=CGFloat(50+(i%6)*360),y=CGFloat(i<6 ? 145:615)
 text(titles[i],x,y,16,true)
 let img=NSImage(contentsOf:source.appendingPathComponent(names[i]+".png"))!
 NSGraphicsContext.current!.imageInterpolation = .none
 img.draw(in:NSRect(x:x,y:y+31,width:216,height:384),from:.zero,operation:.sourceOver,fraction:1,respectFlipped:true,hints:nil)
}
text("A confirm  ·  B up  ·  C down  ·  Hold B back  |  No hardware capture or messaging compatibility claim",50,1065,17)
board.unlockFocus();save(board,"00-overview.jpg")
print("Eleven screen JPEGs and overview saved")
