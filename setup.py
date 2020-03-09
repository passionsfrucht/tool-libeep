from setuptools import setup

setup(
    name="libeep",
    version="0.3",
    description="Object-oriented interface to load eego cnt files",
    url="https://github.com/translationalneurosurgery/libeep",
    author="Robert Smies",
    packages=["libeep"],
    zip_safe=False,
    package_data={"libeep": ["*"]},
)
