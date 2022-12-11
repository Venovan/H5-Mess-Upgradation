from django import forms
from .models import Student


class ProfileForm(forms.ModelForm):
    class Meta:
        model = Student
        fields = ('photo')
