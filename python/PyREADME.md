# How to call the SmartThings core SDK from Python

By default, the core SDK supports C language device apps only.  However, I've created an API wrapper so you can also write device apps in Python.

First, proceed with the complete setup for this RPI setup package (mastersetup) and make sure you have the C-language example device app working (fully onboarded and running).
Before you proceed with Python setup, be sure to exit the C example device app (but don't delete the device from SmartThings mobile app).

## Step 1: Create a python project directory and virtual environment (recommended, but not mandatory) and activate it
```
  mkdir /home/pi/<myproj>
  cd /home/pi/<myproj>
  python3 -m venv venv
  source venv/bin/activate  (type 'deactivate' at command prompt to return to non-virtual environment)
```
## Step 2: Run the setup script from your project directory
This will get all needed files, including installing python modules into your virtual environment, and create a new python shared library: STDK_API.cpython-37m-arm-linux-gnueabihf.so
```
home/pi/rpi-st-device/python/setup
```

## Step 3: Run the example python app
```
python main.py
```


# Writing SmartThings device apps in Python
- You can use main.py as a template.  
- Be sure that the shared object file you built STDK_API.cpython-37m-arm-linux-gnueabihf.so), as well as libiotcore.a, is in your lib path for python to find.
- Add 'from STDevice import \*' to your python script (note that if you are using an IDE, it will complain about the libraries not being found; ignore).
- Use the STDevice class to invoke the SmartThings API.
- If you need to define additional callbacks, you *must* also declare them in iotcorebuild.py (around line 205 with the others), and rebuild the shared object library under your virtual environment:
```
    python iotcorebuild.py
```
- Not every SDK API is covered in the STDevice class at present, but the base ones are there.  If you need others, you can extend the class fairly easily; please consider contributing your enhancements back to this repository
