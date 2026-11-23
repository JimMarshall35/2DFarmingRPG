# run this to populate the sha and build time defines in the WfVersion.h file, only run it in CI.
# and don't touch WfVersion.h except to increment the major minor and revision version numbers
# Written as a python script so it works on windows 
import subprocess
import time

def get_git_sha():
    return subprocess.check_output(["git", "log", "-1", "--format=%H"]).decode().strip()

def get_git_branch():
    return subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"]).decode().strip()

def main():
    data = ""
    with open('./core/include/ZzFX_version.h', 'r') as file:
        data = file.read()
        sha = get_git_sha()
        print(f"sha: {sha}")
        formatted_time = time.strftime("%x-%X", time.gmtime())
        print(f"time: {formatted_time}")
        branch = get_git_branch()
        print(f"branch: {branch}")
        data = data.replace("BUILD_SHA \"<local_build>\"", f"BUILD_SHA \"{sha}\"")
        data = data.replace("BUILD_TIME \"<local_build>\"", f"BUILD_TIME \"{formatted_time}\"")
        data = data.replace("ZZFX_BRANCH \"main\"", f"ZZFX_BRANCH \"{branch}\"")

    with open("./core/include/ZzFX_version.h", "w") as file:
        file.write(data)

main()