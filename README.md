# List Capture Devices for Python OpenCV on Windows
**OpenCV** does not have an API for listing capture devices. The sample shows how to create a Python extension to invoke DirectShow C++ APIs for enumerating capture devices.

I modified a version from [here](https://github.com/yushulx/python-capture-device-list) add serial number for each device.

## Environment
* [Microsoft Windows SDK][0]
* Python 3.8
* OpenCV 3.4.0

NOTE: other versions may also be ok, but I didn't test them.

## How to Run 
1. Configure Visual Studio environment:
    * Visual Studio 2010 (VS10): SET VS90COMNTOOLS=%VS100COMNTOOLS%
    * Visual Studio 2012 (VS11): SET VS90COMNTOOLS=%VS110COMNTOOLS%
    * Visual Studio 2013 (VS12): SET VS90COMNTOOLS=%VS120COMNTOOLS%
    * Visual Studio 2015 (VS14): SET VS90COMNTOOLS=%VS140COMNTOOLS%

    If you are using **Visual Studio 2015**, use the following command:

    ```
    SET VS90COMNTOOLS=%VS140COMNTOOLS%
    ```

2. Add your Windows SDK lib path to **setup.py**:

    ```python
    from distutils.core import setup, Extension

    module_device = Extension('cv_device',
                            sources = ['cv_device.cpp'], 
                            library_dirs=['G:\Program Files\Microsoft SDKs\Windows\v6.1\Lib']   # <== Here, if u don't add it to path, change here.
                        )

    setup (name = 'WindowsDevices',
            version = '1.0',
            description = 'Get device list with DirectShow',
            ext_modules = [module_device])
    ```

3. Build the Python extension

    Python

    ```
    python setup.py build install
    ```

4. Run the app and select a capture device:

    Python 
    ```python
    python test.py
    ```
    ![camera list in Python](screenshot/python-list-device.PNG)

## Blog
[Listing Multiple Cameras for OpenCV-Python on Windows][1]

[0]:https://en.wikipedia.org/wiki/Microsoft_Windows_SDK
[1]:http://www.codepool.biz/multiple-camera-opencv-python-windows.html
