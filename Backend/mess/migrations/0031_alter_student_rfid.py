# Generated by Django 4.1.3 on 2023-01-14 06:11

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('mess', '0030_alter_student_alias_alter_student_permission'),
    ]

    operations = [
        migrations.AlterField(
            model_name='student',
            name='RFID',
            field=models.CharField(blank=True, max_length=15, null=True, unique=True),
        ),
    ]
