class current_level:

    # IR / RED Current Level
    ir_current_level = 7
    red_current_level = 7


    # IR Current
    @classmethod
    def setIrCurrentLevel(cls, ir_current):
        cls.ir_current_level = ir_current

    @classmethod
    def getIrCurrentLevel(cls):
        return cls.ir_current_level


    # RED Current
    @classmethod
    def setRedCurrentLevel(cls, red_current):
        cls.red_current_level = red_current

    @classmethod
    def getRedCurrentLevel(cls):
        return cls.red_current_level