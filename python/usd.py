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

class PrimId:
    id: int

    def __init__(self, id)
        self.id = id

    def inc(self):
        self.id += 1

class Prim:
    id: PrimId
    parent_id: PrimId
    name: str

    def __init__(self, id, parent_id, name):
        self.id = id
        self.parent_id = parent_id
        self.name = name

def flatten_prim(prim: Usd.Prim, parent_id: PrimId, flat_list: list[Prim], current_id: PrimId):
    for child in prim.GetChildren():
        child_prim = Prim(current_id, parent_id, child.GetName())
        flat_list.append(child_prim)
        current_id.inc()
        flatten_prim(child, child_prim.id, flat_list, current_id)

def build_flat_list():
    if _projectStage is not None:
        flat_list: list[Prim] = []
        root: Usd.Prim = _projectStage.GetPseudoRoot()

        current_id = PrimId(1)
        root_prim = Prim(current_id, 0, root.GetName())
        flat_list.append(root_prim)

        current_id.inc()
        flatten_prim(root, root_prim.id, flat_list, current_id)


def add_mesh(stage, primPath):
    prim = stage.DefinePrim("/Library/Brushes/brush_wall_A")
    prim.GetReferences().AddReference(primPath)
    stage.Save()
