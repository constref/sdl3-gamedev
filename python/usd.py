from pxr import Usd, UsdGeom, Tf


def create_project(path):
    project_stage = Usd.Stage.CreateNew(path)
    project_stage.DefinePrim("/Library", UsdGeom.Tokens.Scope)
    project_stage.DefinePrim("/Library/Meshes", UsdGeom.Tokens.Scope)
    project_stage.DefinePrim("/Library/Brushes", UsdGeom.Tokens.Scope)
    world = project_stage.DefinePrim("/World", UsdGeom.Tokens.Xform)
    project_stage.SetDefaultPrim(world)
    return project_stage


def open_project(path):
    project_stage = Usd.Stage.Open(path)
    return project_stage


def save_project(stage: Usd.Stage):
    stage.Save()


def on_objects_changed(self, notice: Usd.Notice.ObjectsChanged, stage: Usd.Stage):
    print("Stage updated")

def listen_on_stage(stage: Usd.Stage):
    return Tf.Notice.Register(Usd.Notice.ObjectsChanged, )

def open_stage(path):
    stage = Usd.Stage.Open(path)
    return stage


class PrimId:
    value: int

    def __init__(self, value):
        self.value = value

    def inc(self):
        self.value += 1


class Prim:
    id: PrimId
    parent_id: PrimId
    name: str
    path: str
    type: str

    def __init__(self, id, parent_id, name, path, type):
        self.id = id
        self.parent_id = parent_id
        self.name = name
        self.path = path
        self.type = type

    def __repr__(self):
        return f"Prim(id={self.id.value}, name={self.name}, type={self.type}, path={self.path} parent_id={self.parent_id.value})"


def flatten_prim(
    prim: Usd.Prim, parent_id: PrimId, flat_list: list[Prim], current_id: PrimId
):
    child: Usd.Prim
    for child in prim.GetChildren():
        child_prim = Prim(
            PrimId(current_id.value),
            PrimId(parent_id.value),
            child.GetName(),
            child.GetPath().pathString,
            child.GetTypeName(),
        )
        flat_list.append(child_prim)
        current_id.inc()
        flatten_prim(child, child_prim.id, flat_list, current_id)


def build_flat_list(stage: Usd.Stage):
    flat_list: list[Prim] = []
    root: Usd.Prim = stage.GetPseudoRoot()

    current_id = PrimId(1)
    root_prim = Prim(
        PrimId(current_id.value),
        PrimId(0),
        root.GetName(),
        root.GetPath().pathString,
        root.GetTypeName(),
    )
    flat_list.append(root_prim)

    current_id.inc()
    flatten_prim(root, root_prim.id, flat_list, current_id)
    return flat_list


def add_mesh(stage: Usd.Stage, asset_path, prim_path):
    asset_stage = Usd.Stage.Open(asset_path)
    prim = asset_stage.GetPrimAtPath(prim_path)

    new_prim = stage.DefinePrim(f"/Library/Meshes/{prim.GetName()}")
    new_prim.GetReferences().AddReference(asset_path, prim_path)
