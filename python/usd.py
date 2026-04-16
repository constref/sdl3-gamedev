from pxr import Usd, UsdGeom, Tf

class StageProxy:
    stage: Usd.Stage
    listener: Tf.Notice.Listener | None

    def __init__(self, stage: Usd.Stage, delegate):
        self.stage = stage
        self.delegate = delegate
        self.listener = Tf.Notice.Register(Usd.Notice.ObjectsChanged, self.on_objects_changed, stage)

    def on_objects_changed(self, notice: Usd.Notice.ObjectsChanged, stage: Usd.Stage):
        path_list = [p.pathString for p in notice.GetResyncedPaths()]
        self.delegate(path_list)

    def revoke(self):
        if self.listener:
            self.listener.Revoke()
            self.listener = None

def create_project(path, delegate):
    stage = Usd.Stage.CreateNew(path)
    stage.DefinePrim("/Library", UsdGeom.Tokens.Scope)
    stage.DefinePrim("/Library/Meshes", UsdGeom.Tokens.Scope)
    stage.DefinePrim("/Library/Brushes", UsdGeom.Tokens.Scope)
    world = stage.DefinePrim("/World", UsdGeom.Tokens.Xform)
    stage.SetDefaultPrim(world)

    return StageProxy(stage, delegate)

def open_project(path, delegate):
    stage = Usd.Stage.Open(path)
    return StageProxy(stage, delegate)

def save_project(stage_proxy: StageProxy):
    stage_proxy.stage.Save()

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

def add_mesh(proxy: StageProxy, asset_path, prim_path):
    asset_stage = Usd.Stage.Open(asset_path)
    prim = asset_stage.GetPrimAtPath(prim_path)

    new_prim = proxy.stage.DefinePrim(f"/Library/Meshes/{prim.GetName()}")
    new_prim.GetReferences().AddReference(asset_path, prim_path)

def add_brush(proxy: StageProxy, mesh_path):
    mesh = proxy.stage.GetPrimAtPath(mesh_path)
    brush = proxy.stage.DefinePrim(f"/Library/Brushes/brush_{mesh.GetName()}")
    brush.GetReferences().AddInternalReference(mesh.GetPath())