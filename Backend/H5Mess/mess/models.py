from django.db import models
from django.core.exceptions import ValidationError

MEAL_CHOICES = [('B', 'Breakfast'),
                ('L', 'Lunch'),
                ('S', 'Snacks'),
                ('D', 'Dinner')]

STATUS = [  ('A', "Allowed"), 
            ('NA', "Not Allowed")]



def ID_valid(value):
    if (len(value) != 9):
        raise ValidationError(("%(value)s is invalid"), params={"value": value})
    else:
        return value

def rename(instance, filename):
    ext = filename.split('.')[-1]
    return '{}.{}'.format(instance.rollNumber, ext)



# Create your models here.
class Student(models.Model):
    name = models.CharField(max_length=50)
    permission = models.CharField(max_length=10, choices=STATUS, default='NA')
    rollNumber = models.CharField(max_length=12, validators=[ID_valid], unique=True)
    roomNumber = models.CharField(max_length=6)
    RFID = models.CharField(max_length=15, blank=True, unique=True)
    photo = models.ImageField(upload_to= rename, default='default.jpg')

    class Meta:
        ordering = ["rollNumber"]


    def __str__(self):
        return self.name


class Meal(models.Model):
    student = models.ForeignKey(Student, on_delete=models.CASCADE)
    type = models.CharField(max_length=5, choices=MEAL_CHOICES)
    weight = models.CharField(max_length=6)
    date = models.DateField()

    class Meta:
        ordering = ["date"]

    def __str__(self):
        return self.student.rollNumber + "/" + str(self.date) + "/" + self.type

