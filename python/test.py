import pprint

from pxr import Usd

import usd


class UsdTester:
    stage: Usd.Stage | None

    def __init__(self):
        self.stage = None

    def main_menu(self):
        print("""
        C. Create Project
        O. Open Project
        """)
        ans = input("Make a selection: ")
        if ans == "C":
            file_path = input("Enter path: ")
            self.stage = usd.create_project(file_path)
            print("Project created")
        elif ans == "O":
            file_path = input("Enter path: ")
            self.stage = usd.open_project(file_path)
        elif ans == "exit":
            return None
        return True

    def project_menu(self):
        if self.stage:
            print("""
            M. Add Mesh
            B. Add Brush
            G. Graph
            """)

            ans = input("Make a selection: ")
            if ans == "info":
                pprint.pprint(self.stage)
            elif ans == "save":
                usd.save_project(self.stage)
                print("Project saved")
            elif ans == "close":
                self.stage = None
            elif ans == "S":
                usd.save_project(self.stage)
                print("Project saved")
            elif ans == "M":
                file_path = input("Enter File Path: ")
                stage = usd.open_stage(file_path)
                prim_list = usd.build_flat_list(stage)
                pprint.pprint(prim_list)
                prim_id = int(input("Enter Prim Id: "))
                prim = prim_list[prim_id - 1]
                pprint.pprint(prim)
                usd.add_mesh(self.stage, file_path, prim.path)

                print("Mesh added")
            elif ans == "B":
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
