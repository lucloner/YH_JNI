/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-25 11:43
 **/

class HelloWorld {


//    static {
//        YH_JNI_main.initJnilibPath();
//        System.loadLibrary("hello");
//    }

    public static void main(String[] args) {
        new HelloWorld().print();
        new HelloWorld01().print();
    }

    public native void print();
}