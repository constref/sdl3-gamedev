import pprint
from typing import Optional

from pxr import Tf, Usd

import usd


class UsdTester:
    stage: Optional[Usd.Stage]
    listener: Optional[Tf.Notice.Listener]

    def __init__(self):
        self.stage = None

    def on_objects_changed(self, notice: Usd.Notice.ObjectsChanged, stage: Usd.Stage):
        print("Stage changed")

    def main_menu(self):
        print("USD Main Menu")
        print("create, open, exit")

        ans = input("Command:> ")
        if ans == "create":
            file_path = input("Create project:> ")
            self.stage = usd.create_project(file_path)
            self.listener = Tf.Notice.Register(
                Usd.Notice.ObjectsChanged, self.on_objects_changed, self.stage
            )
            print("Project created")
        elif ans == "open":
            file_path = input("Open project:> ")
            self.stage = usd.open_project(file_path)
        elif ans == "exit":
            return None
        return True

    def project_menu(self):
        if self.stage:
            print("Project Menu")
            print("addm, addb, info, save, close")

            ans = input("Command:> ")
            if ans == "info":
                pprint.pprint(self.stage)
            elif ans == "save":
                usd.save_project(self.stage)
                print("Project saved")
            elif ans == "close":
                self.stage = None
            elif ans == "addm":
                file_path = input("Enter file path:> ")
                stage = usd.open_stage(file_path)
                prim_list = usd.build_flat_list(stage)
                pprint.pprint(prim_list)
                prim_id = int(input("Enter prim id:> "))
                prim = prim_list[prim_id - 1]
                pprint.pprint(prim)
                usd.add_mesh(self.stage, file_path, prim.path)
                print("Mesh added")
            elif ans == "addb":
                print("Brush added")
            elif ans == "G":
                flat_list = usd.build_flat_list(self.stage)
                pprint.pprint(flat_list)
            elif ans == "exit":
                return None
            return True
        else:
            return None

    def show_menu(self):
        ans = True
        while ans:
            if not self.stage:
                ans = self.main_menu()
            else:
                ans = self.project_menu()


tester: UsdTester = UsdTester()
tester.show_menu()
