# Generated by Django 4.1.1 on 2022-11-02 06:45

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ("mess", "0003_student_status"),
    ]

    operations = [
        migrations.AlterField(
            model_name="student",
            name="status",
            field=models.IntegerField(choices=[(0, "Allowed"), (1, "Not Allowed")]),
        ),
    ]