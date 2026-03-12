from pxr import Usd


def create_stage(name):
    stage = Usd.Stage.CreateInMemory(name)
    return stage


def add_sublayer(stage, path):
    root_layer = stage.GetRootLayer()
    root_layer.subLayerPaths.append(path)
    stage.Save()
