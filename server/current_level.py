class current_level:

    # IR / RED Current Level
    ir_current_level = 7
    red_current_level = 7

    # IR Current
    @classmethod
    def set_current_level(cls, ir_current):
        cls.ir_current_level = ir_current

    @classmethod
    def get_ir_current_level(cls):
        return cls.ir_current_level


    # RED Current
    @classmethod
    def set_red_current_level(cls, red_current):
        cls.red_current_level = red_current

    @classmethod
    def get_red_current_level(cls):
        return cls.red_current_level