import zoitechat


def on_message(event):
    return None


def init():
    user = zoitechat.get_user_info()
    target = user.get("channel") or "#fabulor"
    zoitechat.log(f"Python sample plugin initialised for {user.get('nickname') or 'unknown'}")
    zoitechat.send_message(target, "Hello from the Python sample plugin")
    zoitechat.register_callback("message", on_message)
