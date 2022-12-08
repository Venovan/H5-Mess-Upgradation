from django.db import models
from django.core.exceptions import ValidationError
from django.utils.timezone import now

MEAL_CHOICES = [('B', 'Breakfast'),
                ('L', 'Lunch'),
                ('S', 'Snacks'),
                ('D', 'Dinner')]

STATUS = [  ('A', "Allowed"), 
            ('NA', "Not Allowed")]


MSG_LEVELS = [  ("warning", 'Warning'),
                ("alert", "Alert"),
                ("notify", "Notification"),
                ("info", "Information")]


def ID_valid(value):
    if (len(value) != 9):
        raise ValidationError(("%(value)s is invalid"), params={"value": value})
    else:
        return value

def image_handler(instance, filename):
    ext = filename.split('.')[-1]
    return "/".join(["photos", '{}.{}'.format(instance.rollNumber, ext)])

def menu_handler(instance, filename):
    ext = filename.split('.')[-1]
    return "/".join(["menu", '{}.{}'.format(str(instance.start.day) + str(instance.start.strftime('%b')), ext)])
    


# Create your models here.
class Student(models.Model):
    name = models.CharField(max_length=50)
    alias = models.CharField(max_length=10, blank=True)
    permission = models.CharField(max_length=10, choices=STATUS, default='NA')
    rollNumber = models.CharField(max_length=12, validators=[ID_valid], unique=True)
    roomNumber = models.CharField(max_length=6)
    RFID = models.CharField(max_length=15, blank=True, unique=False, null=True)
    photo = models.ImageField(upload_to=image_handler, default='default.png')

    class Meta:
        ordering = ["rollNumber"]

    def __str__(self):
        return self.name



class Meal(models.Model):
    student = models.ForeignKey(Student, on_delete=models.CASCADE)
    type = models.CharField(max_length=5, choices=MEAL_CHOICES)
    weight = models.CharField(max_length=6, blank=True)
    date = models.DateField()

    class Meta:
        ordering = ["-date", "student__rollNumber"]

    def __str__(self):
        return self.student.rollNumber + "/" + str(self.date) + "/" + self.type + "/" + self.weight



class Announcement(models.Model):
    heading = models.CharField(max_length=100)
    issueDate = models.DateField()
    body = models.TextField(blank=True)
    display = models.BooleanField(default=False)
    link = models.URLField(max_length=200, blank=True)
    level = models.CharField(max_length = 12, choices=MSG_LEVELS, default="info")

    def __str__(self):
        return self.level + str(self.pk)



class Menu(models.Model):
    file = models.FileField(upload_to=menu_handler)
    start = models.DateField(auto_now=False, default=now)

    def __str__(self):
        return str(self.start.day) + str(self.start.strftime('%b')) 
    
    class Meta:
        ordering = ["start"]