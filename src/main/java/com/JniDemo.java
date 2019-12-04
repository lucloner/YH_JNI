package com;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-27 15:52
 **/
public class JniDemo {
    static {
        System.loadLibrary("RectifyUnlineDll");
    }

    public native static double ImageRecify(String SrcPath, String DstPath);

    public native static void RemoveUnline(String SrcPath, String DstPath, int StartX, int StartY, int EndX, int EndY);

    public native static void set(int i);

    public native static int get();
}
