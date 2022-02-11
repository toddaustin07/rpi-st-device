# How to call the SmartThings core SDK from Python

By default, the core SDK supports C language device apps only.  However, you can create an API wrapper so you can write device apps in Python.

- Note:  This currently works only on 32-bit Raspberry Pi OS. 

First, proceed with the complete setup for this RPI setup package (mastersetup) and make sure you have the C-language example device app working (fully onboarded and running).
Before you proceed with Python setup, you can exit the C example device app if it is running, **but don't delete the test device from SmartThings mobile app**.

## Step 1: Create a python project directory and virtual environment (VE is recommended, but not mandatory), and activate the VE
```
  mkdir ~/<myproj>
  cd ~/<myproj>
  python3 -m venv venv
  source venv/bin/activate  (type 'deactivate' at command prompt to return to non-virtual environment)
```
## Step 2: Run the setup script from your project directory
This will get all needed files, including installing python modules into your virtual environment, and create a new python shared library: STDK_API.cpython-37m-arm-linux-gnueabihf.so
```
~/rpi-st-device/python/setup
```

## Step 3: Run the example python app
```
python pyexample.py
```


# Writing SmartThings device apps in Python
- You can use pyexample.py as a template.  
- Be sure that the shared object file you built (STDK_API.cpython-37m-arm-linux-gnueabihf.so), as well as libiotcore.a, is in your lib path for python to find.
- Add 'from STDevice import \*' to your python script (note that if you are using an IDE, it will complain about the libraries not being found; ignore that).
- Use the STDevice class methods to invoke the SmartThings API within your device app (you can have only one instance of a STDevice object with the current code).
- If you need to define additional callbacks for your device app, you *must* also declare them in iotcorebuild.py (insert after line 14), and rebuild the shared object library under your virtual environment:
```
    cd ~/<myproj>
    source venv/bin/activate
    nano iotcorebuild.py      <--- add your callback declarations
    python iotcorebuild.py
```
- Not every SDK API is covered in the STDevice class at present, but the base ones are there.  If you need others, you can extend the class fairly easily; please consider contributing your enhancements back to this repository
