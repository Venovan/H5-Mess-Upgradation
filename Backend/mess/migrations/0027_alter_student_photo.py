# Generated by Django 4.0.6 on 2022-12-06 11:32

from django.db import migrations, models
import mess.models


class Migration(migrations.Migration):

    dependencies = [
        ('mess', '0026_alter_student_roomnumber'),
    ]

    operations = [
        migrations.AlterField(
            model_name='student',
            name='photo',
            field=models.ImageField(default='avatar.jpg', upload_to=mess.models.image_handler),
        ),
    ]
