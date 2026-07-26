# Fabulor-Name: Simple Python Greeter
# Fabulor-Version: 1.0.0
# Fabulor-Description: Minimal simple Python add-on.

import zoitechat


def init():
    user = zoitechat.get_user_info()
    nickname = user.get("nickname") or "unknown"
    zoitechat.log(f"Hello, {nickname}. Simple Python add-on ready.")
