package com;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-27 15:29
 **/
public class RemoveBlackBorderDll {

//    static {
//        System.loadLibrary("RemoveBlackBorder");
//    }

    public native static void RemoveBlackBorder(String SrcPath, String DstPath);
}
