// Draws the app icon: Pac-Man chasing dots on a dark rounded tile.
// usage: swift make_icon.swift out.png   (1024x1024)
import AppKit

let size: CGFloat = 1024
let image = NSImage(size: NSSize(width: size, height: size))
image.lockFocus()

// Background: dark navy rounded square with the maze-blue double outline.
let inset: CGFloat = 100
let tile = NSRect(x: inset, y: inset, width: size - 2 * inset, height: size - 2 * inset)
let bg = NSBezierPath(roundedRect: tile, xRadius: 185, yRadius: 185)
NSColor(calibratedRed: 0.03, green: 0.03, blue: 0.10, alpha: 1).setFill()
bg.fill()
let blue = NSColor(calibratedRed: 0.33, green: 0.33, blue: 0.95, alpha: 1)
blue.setStroke()
for d: CGFloat in [26, 52] {
    let p = NSBezierPath(roundedRect: tile.insetBy(dx: d, dy: d), xRadius: 185 - d, yRadius: 185 - d)
    p.lineWidth = 9
    p.stroke()
}

// Dots and a power dot.
NSColor(calibratedRed: 1.0, green: 0.85, blue: 0.62, alpha: 1).setFill()
for x: CGFloat in [610, 700] {
    NSBezierPath(ovalIn: NSRect(x: x - 22, y: 512 - 22, width: 44, height: 44)).fill()
}
NSBezierPath(ovalIn: NSRect(x: 790 - 46, y: 512 - 46, width: 92, height: 92)).fill()

// Pac-Man, mouth open to the right.
let center = NSPoint(x: 400, y: 512)
let pac = NSBezierPath()
pac.move(to: center)
pac.appendArc(withCenter: center, radius: 225, startAngle: 35, endAngle: 325)
pac.close()
NSColor(calibratedRed: 1.0, green: 0.93, blue: 0.0, alpha: 1).setFill()
pac.fill()

image.unlockFocus()
let rep = NSBitmapImageRep(data: image.tiffRepresentation!)!
try! rep.representation(using: .png, properties: [:])!.write(to: URL(fileURLWithPath: CommandLine.arguments[1]))
