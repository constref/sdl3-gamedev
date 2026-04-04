from pxr import Usd, UsdGeom


def create_stage(path):
    stage = Usd.Stage.CreateNew(path)
    stage.DefinePrim("/Library", UsdGeom.Tokens.Scope)
    stage.DefinePrim("/Library/Meshes", UsdGeom.Tokens.Scope)
    stage.DefinePrim("/Library/Brushes", UsdGeom.Tokens.Scope)
    stage.DefinePrim("/World", UsdGeom.Tokens.Xform)
    stage.Save()
    return stage


def add_mesh(stage, primPath):
    prim = stage.DefinePrim("/Library/Brushes/brush_wall_A")
    prim.GetReferences().AddReference(primPath)
    stage.Save()
