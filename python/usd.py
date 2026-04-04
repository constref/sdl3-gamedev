from pxr import Usd


def create_stage(path):
    stage = Usd.Stage.CreateNew(path)


create_stage("coolstage.usda")
