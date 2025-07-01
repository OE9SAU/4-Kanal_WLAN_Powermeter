import FreeCAD, Part
from FreeCAD import Vector

# Parameter
platinen_laenge = 155
platinen_breite = 52
lochabstand_x = 142
lochabstand_y = 40
abstandshalter_hoehe = 10
wandstaerke = 3
deckel_hoehe = 3
loch_durchmesser = 3.2
deckel_schraubabstand = 10

gehause_laenge = platinen_laenge + 2*wandstaerke
gehause_breite = platinen_breite + 2*wandstaerke
boden_hoehe = 30
gehause_hoehe = boden_hoehe  # Deckel liegt auf Bodenhöhe

bohrloch_radius = loch_durchmesser / 2

# Neues Dokument anlegen
doc = FreeCAD.newDocument("Deckel")

# Deckel-Box (auf Bodenhöhe)
deckel = Part.makeBox(gehause_laenge, gehause_breite, deckel_hoehe, Vector(0, 0, gehause_hoehe))

# Abstandshalter auf dem Deckel (oben drauf)
abstandshalter_z = gehause_hoehe + deckel_hoehe

abstandshalter_pos = [
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2,
           wandstaerke + (platinen_breite - lochabstand_y)/2,
           abstandshalter_z),
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2 + lochabstand_x,
           wandstaerke + (platinen_breite - lochabstand_y)/2,
           abstandshalter_z),
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2,
           wandstaerke + (platinen_breite - lochabstand_y)/2 + lochabstand_y,
           abstandshalter_z),
    Vector(wandstaerke + (platinen_laenge - lochabstand_x)/2 + lochabstand_x,
           wandstaerke + (platinen_breite - lochabstand_y)/2 + lochabstand_y,
           abstandshalter_z)
]

abstandshalter_radius = 5

abstandshalter = []
for pos in abstandshalter_pos:
    cyl = Part.makeCylinder(abstandshalter_radius, abstandshalter_hoehe, pos)
    abstandshalter.append(cyl)

for cyl in abstandshalter:
    deckel = deckel.fuse(cyl)

# Schraublöcher im Deckel (auf Bodenhöhe)
deckel_schraub_pos = [
    Vector(deckel_schraubabstand, deckel_schraubabstand, gehause_hoehe),
    Vector(gehause_laenge - deckel_schraubabstand, deckel_schraubabstand, gehause_hoehe),
    Vector(gehause_laenge - deckel_schraubabstand, gehause_breite - deckel_schraubabstand, gehause_hoehe),
    Vector(deckel_schraubabstand, gehause_breite - deckel_schraubabstand, gehause_hoehe)
]

for pos in deckel_schraub_pos:
    loch = Part.makeCylinder(bohrloch_radius, deckel_hoehe + abstandshalter_hoehe + 1, pos)
    deckel = deckel.cut(loch)

# Deckel anzeigen
Part.show(deckel)

# Ansicht anpassen (funktioniert nur in der FreeCAD GUI)
try:
    import FreeCADGui
    FreeCADGui.SendMsgToActiveView("ViewFit")
    FreeCADGui.activeDocument().activeView().viewAxometric()
except ImportError:
    pass  # Keine GUI vorhanden

# Datei speichern - unbedingt Pfad anpassen!
filename = r"C:\Users\martin.sauter\Desktop\Gehause_Deckel.FCStd"
doc.saveAs(filename)
print(f"Deckel-Datei gespeichert als {filename}")
