import FreeCAD, Part
from FreeCAD import Vector

# Parameter
platinen_laenge = 155
platinen_breite = 52
lochabstand_x = 142
lochabstand_y = 40
boden_hoehe = 30
wandstaerke = 3
loch_durchmesser = 3.2

gehause_laenge = platinen_laenge + 2*wandstaerke
gehause_breite = platinen_breite + 2*wandstaerke

bohrloch_radius = loch_durchmesser / 2
bohrloch_hoehe = boden_hoehe + 1  # Bohrloch tief genug für Boden

doc = FreeCAD.newDocument("Boden")

boden_outer = Part.makeBox(gehause_laenge, gehause_breite, boden_hoehe)
boden_inner = Part.makeBox(
    gehause_laenge - 2*wandstaerke,
    gehause_breite - 2*wandstaerke,
    boden_hoehe - wandstaerke,
    Vector(wandstaerke, wandstaerke, wandstaerke)
)
boden_hohl = boden_outer.cut(boden_inner)

# Positionen der Bohrlöcher (unten im Boden)
bohrloch_pos = [
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2,
           wandstaerke + (platinen_breite - lochabstand_y)/2,
           0),
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2 + lochabstand_x,
           wandstaerke + (platinen_breite - lochabstand_y)/2,
           0),
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2,
           wandstaerke + (platinen_breite - lochabstand_y)/2 + lochabstand_y,
           0),
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2 + lochabstand_x,
           wandstaerke + (platinen_breite - lochabstand_y)/2 + lochabstand_y,
           0)
]

for pos in bohrloch_pos:
    loch = Part.makeCylinder(bohrloch_radius, bohrloch_hoehe, pos)
    boden_hohl = boden_hohl.cut(loch)

Part.show(boden_hohl)

filename = r"C:\Users\martin.sauter\Desktop\Gehause_Boden.FCStd"
doc.saveAs(filename)
print(f"Boden-Datei gespeichert als {filename}")
