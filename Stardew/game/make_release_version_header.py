# run this to populate the sha and build time defines in the WfVersion.h file, only run it in CI.
# and don't touch WfVersion.h except to increment the major minor and revision version numbers
# Written as a python script so it works on windows 
import subprocess
import time
import argparse

def get_git_sha():
    return subprocess.check_output(["git", "log", "-1", "--format=%H"]).decode().strip()

def get_git_branch():
    return subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"]).decode().strip()


def do_cmd_args():
    parser = argparse.ArgumentParser(
                    prog='Stamper',
                    description='Stamp a release version file',
                    epilog='Jim Marshall - 2025')
    parser.add_argument("--baked", action="store_true")
    parser.add_argument("--platform", type=str)
    args = parser.parse_args()
    return args

def main():
    args = do_cmd_args()
    data = ""
    with open('./include/WfVersion.h', 'r') as file:
        data = file.read()
        sha = get_git_sha()
        print(f"sha: {sha}")
        formatted_time = time.strftime("%x-%X", time.gmtime())
        print(f"time: {formatted_time}")
        branch = get_git_branch()
        print(f"branch: {branch}")
        data = data.replace("BUILD_SHA \"<local_build>\"", f"BUILD_SHA \"{sha}{"-baked" if args.baked else ""}-{args.platform}\"")
        data = data.replace("BUILD_TIME \"<local_build>\"", f"BUILD_TIME \"{formatted_time}\"")
        data = data.replace("WF_BRANCH \"main\"", f"WF_BRANCH \"{branch}\"")

    with open("./include/WfVersion.h", "w") as file:
        file.write(data)

main()