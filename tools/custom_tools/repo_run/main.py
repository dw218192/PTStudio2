"""
Allows easy running of the executable targets built in this repo.

To use, cd to the root of the repo and run:
    ./repo.{shell_ext} run [target]
"""
import argparse
import pathlib as pb
import omni.repo.man as repo
import subprocess as sp

def setup_args(parser: argparse.ArgumentParser):
    parser.add_argument("target", type=str, help="The target to run")
    parser.add_argument("-d", "--debug", action="store_true", help="Enable debug mode", required=False, default=False)
    parser.add_argument("-r", "--release", action="store_true", help="Enable release mode", required=False, default=False)
    parser.add_argument('-a', '--exe-args', nargs=argparse.REMAINDER, help="Extra arguments to pass to the executable", required=False, default=[])
    parser.description = "Runs a build target"

def setup_repo_tool(parser: argparse.ArgumentParser, config: dict):
    setup_args(parser)

    build_folder = config["repo"]["folders"]["build"]
    platform_host = repo.get_host_platform()
    build_folder_path = pb.Path(build_folder) / platform_host

    def run_repo_tool(options, config):
        if options.debug and options.release:
            raise ValueError("Cannot specify both debug and release modes")
        elif options.debug:
            target_build_config = "Debug"
            print("Debug mode")
        elif options.release:
            target_build_config = "Release"
            print("Release mode")
        else:
            target_build_config = "Release"
            print("No mode specified, running release mode")

        exe_ext = repo.resolve_tokens("${exe_ext}")
        for file in (build_folder_path / target_build_config).glob(f"*{exe_ext}"):
            if file.stem == options.target or file.name == options.target:
                print(f"Running {file.stem}")
                sp.run([file.resolve(), *options.exe_args])
                return
        print(f"No executable found for target {options.target} with build config {target_build_config}")

    return run_repo_tool