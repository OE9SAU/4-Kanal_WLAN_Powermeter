import FreeCAD, Part
from FreeCAD import Vector
import os

# Parameter
platinen_laenge = 155
platinen_breite = 52
lochabstand_x = 142
lochabstand_y = 40
boden_hoehe = 30
wandstaerke = 3
loch_durchmesser = 3.2

gehause_laenge = platinen_laenge + 2 * wandstaerke
gehause_breite = platinen_breite + 2 * wandstaerke

bohrloch_radius = loch_durchmesser / 2
bohrloch_hoehe = boden_hoehe + 1  # etwas tiefer, damit Loch komplett durchgeht

# Neues Dokument
doc = FreeCAD.newDocument("Boden")

# Boden erzeugen (hohler Quader)
boden_outer = Part.makeBox(gehause_laenge, gehause_breite, boden_hoehe)
boden_inner = Part.makeBox(
    gehause_laenge - 2 * wandstaerke,
    gehause_breite - 2 * wandstaerke,
    boden_hoehe - wandstaerke,
    Vector(wandstaerke, wandstaerke, wandstaerke)
)
boden_hohl = boden_outer.cut(boden_inner)

# Bohrlöcher
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

# Aussparung an der Rückseite (Y = Rückkante), nur oberer Teil ausgeschnitten
aussparung_laenge = gehause_laenge
aussparung_tiefe = wandstaerke
aussparung_hoehe = boden_hoehe - 20  # nur oberer Bereich wird ausgeschnitten

aussparung = Part.makeBox(aussparung_laenge, aussparung_tiefe, aussparung_hoehe)
aussparung.translate(Vector(0, gehause_breite - aussparung_tiefe, 20))  # 20 mm stehen lassen unten

# Vom Boden abziehen
gesamt = boden_hohl.cut(aussparung)

# Anzeigen
Part.show(gesamt)

# Datei speichern
desktop = os.path.join(os.path.expanduser("~"), "Desktop")
filename = os.path.join(desktop, "Gehause_Boden.FCStd")
os.makedirs(os.path.dirname(filename), exist_ok=True)
doc.saveAs(filename)

print(f"Boden-Datei mit rückseitiger Aussparung (20 mm Restboden) gespeichert als: {filename}")
