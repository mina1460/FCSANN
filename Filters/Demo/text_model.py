class Result:
    def __init__(self, index, distance):
        self.index = index
        self.distance = distance

    def __init__(self, index, distance, text, f1, f2, f3, f4, id, product):
        self.index = index
        self.distance = distance
        self.text = text
        self.f1 = f1
        self.f2 = f2
        self.f3 = f3
        self.f4 = f4
        self.id = id
        self.product = product

    def __str__(self):
        return (
            str(self.index)
            + "-"
            + self.text
            + "\n"
            + str(self.f1)
            + "\t"
            + str(self.f2)
            + "\t"
            + str(self.f3)
            + "\t"
            + str(self.f4)
            + "\n"
            + str(self.distance)
        )

    def __repr__(self):
        return self.__str__()
