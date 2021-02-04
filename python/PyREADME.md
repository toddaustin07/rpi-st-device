# How to call the SmartThings core SDK from Python

## Step 1: Create a virtual environment (recommended, but not mandatory) and activate it
Within your project directory:
```
  python3 -m venv venv
  source venv/bin/activate  ('deactivate' when done)
```
## Step 2: Install required Python modules
```
python3 -m pip3 install -r requirements.txt
```
## Step 3: Get core SDK library file: libiotcore.a
Either use the pre-built one supplied in this directory, or one you've built yourself

## Step 4: Create Python library for core SDK
python iotcorebuild.py

## Step 5: Setup test switch device on SmartThings
See instructions for 
