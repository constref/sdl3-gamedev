import pprint
from typing import Optional

from pxr import Tf, Usd

import usd
from usd import StageProxy


class UsdTester:
    proxy: Optional[StageProxy]

    def __init__(self):
        self.proxy = None

    def on_objects_changed(self, list):
        pprint.pprint(list)

    def main_menu(self):
        print("\nUSD Main Menu")
        print("create, open, exit")

        ans = input("Command:> ")
        if ans == "create":
            file_path = input("Create project:> ")
            stage = usd.create_project(file_path)
            self.proxy = usd.create_proxy(stage)
            usd.register_listener(self.proxy, self.on_objects_changed)
            print("Project created")
        elif ans == "open":
            file_path = input("Open project:> ")
            stage = usd.open_project(file_path)
            self.proxy = usd.create_proxy(stage)
            usd.register_listener(self.proxy, self.on_objects_changed)
            print("Project opened")
        elif ans == "exit":
            return None
        return True

    def project_menu(self):
        if self.proxy:
            print("\nProject Menu")
            print("addm, addb, flat, info, save, close")

            ans = input("Command:> ")
            if ans == "info":
                pprint.pprint(self.proxy.stage)
            elif ans == "save":
                usd.save_project(self.proxy)
                print("Project saved")
            elif ans == "close":
                self.proxy = None
            elif ans == "addm":
                # add mesh command
                file_path = input("Enter file path:> ")
                stage = usd.open_stage(file_path)
                prim_list = usd.build_flat_list(stage)
                pprint.pprint(prim_list)
                prim_id = int(input("Enter prim id:> "))
                prim = prim_list[prim_id - 1]
                pprint.pprint(prim)
                usd.add_mesh(self.proxy, file_path, prim.path)
                print("Mesh added")
            elif ans == "addb":
                # add brush command
                meshes_prim = self.proxy.stage.GetPrimAtPath("/Library/Meshes")
                meshes = meshes_prim.GetChildren()
                for idx, path in enumerate(meshes):
                    print(f"{idx}:{path}")
                mesh_index = int(input("Enter mesh index:> "))
                usd.add_brush(self.proxy, meshes[mesh_index].GetPath().pathString)
                print("Brush added")
            elif ans == "flat":
                flat_list = usd.build_flat_list(self.proxy.stage)
                pprint.pprint(flat_list)
            elif ans == "exit":
                return None
            return True
        else:
            return None

    def show_menu(self):
        ans = True
        while ans:
            if not self.proxy:
                ans = self.main_menu()
            else:
                ans = self.project_menu()


tester: UsdTester = UsdTester()
tester.show_menu()
