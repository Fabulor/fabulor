# Fabulor-Name: Simple Python Greeter
# Fabulor-Version: 1.0.0
# Fabulor-Description: Minimal simple Python add-on.

import fabulor


def init():
    user = fabulor.get_user_info()
    nickname = user.get("nickname") or "unknown"
    fabulor.log(f"Hello, {nickname}. Simple Python add-on ready.")
