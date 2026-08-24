__module_name__ = "Simple Python Greeter"
__module_version__ = "1.0.0"
__module_description__ = "Minimal simple Python add-on."

import fabulor


def init():
    user = fabulor.get_user_info()
    nickname = user.get("nickname") or "unknown"
    fabulor.log(f"Hello, {nickname}. Simple Python add-on ready.")


init()
