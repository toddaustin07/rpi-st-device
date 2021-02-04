# How to call the SmartThings core SDK from Python

First, proceed with the complete setup for this package and make sure you have the C-language example app working.
Exit the C example device app.

## Step 1: Create python project directory and virtual environment (recommended, but not mandatory) and activate it
Within your python project directory:
```
  python3 -m venv venv
  source venv/bin/activate  ('deactivate' when done)
```
## Step 2: Install required Python modules
```
python3 -m pip3 install -r requirements.txt
```
## Step 3: Get core SDK library file: libiotcore.a
Either use the pre-built one supplied in this package python directory, or one you've built yourself from ~/st-device-sdk-c/output

## Step 4: Create Python library for core SDK
```
python iotcorebuild.py
```
This will create a new python shared library: STDK_API.cpython-37m-arm-linux-gnueabihf.so

## Step 5: Copy files from C example directory into your python project directory
Note: To avoid re-onboarding process, we'll assume you have gotten the C-language example app working and we'll steal the files from that: (~/st-device-sdk-c/example).  Make sure you have terminated that app first.
```
device_info.json
onboarding_config.json
Provisioning files:  DeviceID, WifiProvStatus, ServerURL, ServerPort, Label, IotAPSSID, IotAPBSSID, IotAPAuthType, CloudProvStatus, MiscInfo
```

## Step 6: Run the example python app
```
python main.py
```

## Creating Python device apps
- You can use the main.py as a template.  
- Be sure that the shared object file you build as well as libiocore.a is in your lib path for python to find.
- IMPORT STDevice.py in your python app (note that if you are using an IDE, it will complain about the libraries not being found; ignore).
- Use the STDevice class to access the SmartThings API.
- If you need to define additional callbacks, you must also declare them in iotcorebuild.py (around line 205 with the others), and rebuild the library (python iotcorebuild.py).
- Not every SDK API is provided at this time, but the base ones are there.  If you need others, you can probably extend the class fairly easily.
