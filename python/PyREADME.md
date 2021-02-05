# How to call the SmartThings core SDK from Python

By default, the core SDK supports C language device apps only.  However, I've created an API wrapper so you can also write device apps in Python.

First, proceed with the complete setup for this RPI setup package (mastersetup) and make sure you have the C-language example device app working (fully onboarded and running).
Before you proceed with Python setup, be sure to exit the C example device app (but don't delete the device from SmartThings mobile app).

## Step 1: Create a python project directory and virtual environment (recommended, but not mandatory) and activate it
Within your python project directory:
```
  mkdir /home/pi/myproj
  cd /home/pi/myproj
  python3 -m venv venv
  source venv/bin/activate  (type 'deactivate' at command prompt to return to non-virtual environment)
```
## Step 2: Install required Python modules
```
python -m pip3 install -r requirements.txt
```
## Step 3: Get core SDK library file: libiotcore.a
Either use the pre-built one supplied in this package python directory, or one you've built yourself from ~/st-device-sdk-c/output

## Step 4: Create Python library for core SDK
```
python iotcorebuild.py
```
This will create a new python shared library: STDK_API.cpython-37m-arm-linux-gnueabihf.so

## Step 5: Copy files from C example directory into your python project directory
Note: To avoid re-onboarding the test device, we'll assume you have gotten the C-language example app working and we'll steal the files from that (~/st-device-sdk-c/example) by running a bash script to copy the needed files:
```
getprovfiles
```

## Step 6: Run the example python app
```
python main.py
```

## Creating Python device apps
- You can use the main.py as a template.  
- Be sure that the shared object file you built, as well as libiotcore.a, is in your lib path for python to find.
- 'import STDevice' in your python app (note that if you are using an IDE, it will complain about the libraries not being found; ignore).
- Use the STDevice class to access the SmartThings API.
- If you need to define additional callbacks, you must also declare them in iotcorebuild.py (around line 205 with the others), and rebuild the shared object library
- Not every SDK API is provided at this time, but the base ones are there.  If you need others, you can extend the class fairly easily; please consider contributing your enhancements back to this repository
