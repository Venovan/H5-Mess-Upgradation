# Generated by Django 4.1.3 on 2022-12-06 11:58

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('mess', '0025_alter_meal_weight'),
    ]

    operations = [
        migrations.AlterModelOptions(
            name='meal',
            options={'ordering': ['date', 'student__rollNumber']},
        ),
    ]