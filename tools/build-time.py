"""Generate a UTC build-time floor for offline RTC initialization."""
Import("env")
from pathlib import Path
import time
folder = Path(env.subst("$BUILD_DIR")) / "generated"
folder.mkdir(parents=True, exist_ok=True)
(folder / "BuildTime.h").write_text("#pragma once\n#define STICK_BUILD_EPOCH %dLL\n" % int(time.time()))
env.Append(CPPPATH=[str(folder)])
