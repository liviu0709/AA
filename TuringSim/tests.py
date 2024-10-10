TESTS = {
        "*12:34*34:12*21:43*43:21*": (True, True, True),
        "*12:34*12:34*12:34*12:34*": (True, False, False),
        "*11:11*22:22*33:33*44:44*": (False, True, False),
        "*12:12*34:34*12:12*34:34*": (False, False, True),
        "*11:11*11:11*11:11*11:11*": (False, False, False),
        "*32:14*41:23*24:31*13:42*": (True, True, True),
        "*32:14*23:41*31:42*12:43*": (True, False, False),
        "*11:14*32:32*23:43*44:21*": (False, True, False),
        "*23:32*14:14*21:14*34:32*": (False, False, True),
        "*12:34*12:34*12:24*12:34*": (False, False, False),
        "*14:23*23:14*31:42*42:31*": (True, True, True),
        "*24:13*13:42*21:34*43:12*": (True, False, True),
        "*42:24*13:31*21:12*34:43*": (False, True, True),
        "*43:12*34:21*12:34*21:43*": (True, True, False),
        "*13:34*42:31*21:43*34:12*": (False, False, False),
        "*42:13*13:42*21:34*34:21*": (True, True, True),
        "*41:23*41:23*41:23*41:23*": (True, False, False),
        "*12:31*34:23*23:42*24:44*": (False, False, False),
        "*14:24*42:31*34:12*21:43*": (False, False, False),
        "*33:21*21:34*42:13*13:42*": (False, False, False),
        "*13:24*42:31*21:43*34:12*": (True, True, True),
        "*43:12*21:34*13:42*23:41*": (True, False, False),
        "*41:21*13:43*24:12*32:34*": (False, True, False),
        "*34:34*12:12*34:34*12:12*": (False, False, True),
        "*24:23*32:41*21:34*43:12*": (False, False, False),
        "*34:21*21:34*13:42*42:13*": (True, True, True),
        "*43:21*43:21*43:21*43:21*": (True, False, False),
        "*21:43*34:32*13:24*42:31*": (False, False, False),
        "*42:31*31:24*43:12*21:34*": (True, False, True),
        "*43:23*23:41*34:12*12:34*": (False, False, False),
}
