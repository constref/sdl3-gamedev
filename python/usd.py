from pxr import Usd, UsdGeom, Tf, Sdf

class StageProxy:
    stage: Usd.Stage
    listener: Tf.Notice.Listener | None

    def __init__(self, stage: Usd.Stage):
        self.stage = stage
        self.delegate = None

    def on_objects_changed(self, notice: Usd.Notice.ObjectsChanged, stage: Usd.Stage):
        if self.delegate:
            path_list = [p.pathString for p in notice.GetResyncedPaths()]
            self.delegate(path_list)

    def revoke(self):
        if self.listener:
            self.listener.Revoke()
            self.listener = None

    def get_prim(self, path):
        prim = self.stage.GetPrimAtPath(path)
        return Prim(None, None, prim.GetName(), path, prim.GetTypeName())

def create_project(path):
    stage = Usd.Stage.CreateNew(path)
    stage.DefinePrim("/Library", UsdGeom.Tokens.Scope)
    stage.DefinePrim("/Library/Meshes", UsdGeom.Tokens.Scope)
    stage.DefinePrim("/Library/Brushes", UsdGeom.Tokens.Scope)
    world = stage.DefinePrim("/World", UsdGeom.Tokens.Xform)
    stage.SetDefaultPrim(world)
    return stage

def open_project(path):
    return Usd.Stage.Open(path)

def create_proxy(stage: Usd.Stage):
    return StageProxy(stage)

def register_listener(proxy: StageProxy, delegate):
    proxy.delegate = delegate
    proxy.listener = Tf.Notice.Register(Usd.Notice.ObjectsChanged, proxy.on_objects_changed, proxy.stage)

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
    mesh_path = f"/Library/Meshes/{prim.GetName()}"

    with Sdf.ChangeBlock():
        layer = proxy.stage.GetEditTarget().GetLayer()
        spec = Sdf.CreatePrimInLayer(layer, mesh_path)
        ref = Sdf.Reference(asset_path, prim_path)
        spec.referenceList.Add(ref)

def add_brush(proxy: StageProxy, mesh_path):
    mesh = proxy.stage.GetPrimAtPath(mesh_path)
    brush = proxy.stage.DefinePrim(f"/Library/Brushes/brush_{mesh.GetName()}")
    brush.GetReferences().AddInternalReference(mesh.GetPath())