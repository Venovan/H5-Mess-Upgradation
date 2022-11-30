# Generated by Django 4.1.3 on 2022-11-18 10:42

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('mess', '0012_alter_student_rfid'),
    ]

    operations = [
        migrations.CreateModel(
            name='Announcement',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('body', models.TextField(blank=True)),
                ('display', models.BooleanField(default=False)),
                ('level', models.CharField(choices=[('warning', 'Warning'), ('alert', 'Alert'), ('notify', 'Notification'), ('info', 'Information')], default='info', max_length=12)),
            ],
        ),
        migrations.AlterField(
            model_name='student',
            name='permission',
            field=models.CharField(choices=[('A', 'Allowed'), ('NA', 'Not Allowed')], default='NA', max_length=10),
        ),
    ]