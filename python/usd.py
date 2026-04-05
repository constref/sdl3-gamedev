from pxr import Usd, UsdGeom

_projectStage: Usd.Stage | None = None


def create_project(path):
    global _projectStage
    _projectStage = Usd.Stage.CreateNew(path)
    _projectStage.DefinePrim("/Library", UsdGeom.Tokens.Scope)
    _projectStage.DefinePrim("/Library/Meshes", UsdGeom.Tokens.Scope)
    _projectStage.DefinePrim("/Library/Brushes", UsdGeom.Tokens.Scope)
    _projectStage.DefinePrim("/World", UsdGeom.Tokens.Xform)
    _projectStage.Save()
    return _projectStage


def save_project():
    if _projectStage is not None:
        _projectStage.Save()


def close_project():
    _projectStage = None


def add_mesh(stage, primPath):
    prim = stage.DefinePrim("/Library/Brushes/brush_wall_A")
    prim.GetReferences().AddReference(primPath)
    stage.Save()
