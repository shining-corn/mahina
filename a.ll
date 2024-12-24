; ModuleID = 'test.txt'
source_filename = "test.txt"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@0 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@1 = private unnamed_addr constant [3 x i8] c"\0A\0A\00", align 1
@2 = private unnamed_addr constant [10 x i8] c"fizzbuzz \00", align 1
@3 = private unnamed_addr constant [6 x i8] c"buzz \00", align 1
@4 = private unnamed_addr constant [6 x i8] c"fizz \00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@6 = private unnamed_addr constant [3 x i8] c"\0A\0A\00", align 1

declare i8* @malloc(i64)

declare i32 @printf(i8*, ...)

define [2 x i1] @testArray([2 x i32] %0, [2 x i1] %1, [2 x i8*] %2) {
  %4 = alloca [2 x i32]
  %5 = alloca [2 x i1]
  %6 = alloca [2 x i32]
  %7 = alloca [2 x i32]
  %8 = alloca [3 x i32]
  %9 = alloca [2 x [2 x i32]]
  %10 = alloca [3 x [2 x i64]]
  %11 = alloca [2 x [2 x i64]]
  %12 = alloca [2 x i64]
  %13 = alloca [3 x float]
  %14 = alloca [3 x double]
  %15 = alloca [3 x float]
  %16 = alloca [3 x double]
  %17 = alloca [3 x double]
  %18 = alloca [3 x double]
  %19 = alloca [2 x i32]
  %20 = alloca [2 x i1]
  %21 = alloca [2 x i1]
  %22 = alloca [2 x i32]
  %23 = alloca [2 x i1]
  %24 = alloca [2 x i8*]
  %25 = alloca [2 x i32]
  %26 = alloca [2 x i1]
  %27 = alloca [2 x i8*]
  store [2 x i8*] %2, [2 x i8*]* %27
  store [2 x i1] %1, [2 x i1]* %26
  store [2 x i32] %0, [2 x i32]* %25
  store [2 x i8*] %2, [2 x i8*]* %24
  store [2 x i1] %1, [2 x i1]* %23
  store [2 x i32] %0, [2 x i32]* %22
  store [2 x i1] [i1 false, i1 true], [2 x i1]* %21
  store [2 x i1] [i1 true, i1 false], [2 x i1]* %20
  store [2 x i32] [i32 1, i32 2], [2 x i32]* %19
  store [3 x double] [double 1.000000e+00, double 2.000000e+00, double 3.000000e+00], [3 x double]* %18
  store [3 x double] [double 1.000000e+00, double 2.000000e+00, double 3.000000e+00], [3 x double]* %17
  store [3 x double] [double 1.000000e+00, double 2.000000e+00, double 3.000000e+00], [3 x double]* %16
  store [3 x float] [float 1.000000e+00, float 2.000000e+00, float 3.000000e+00], [3 x float]* %15
  store [3 x double] zeroinitializer, [3 x double]* %14
  store [3 x float] zeroinitializer, [3 x float]* %13
  store [2 x i64] [i64 111, i64 8589934592], [2 x i64]* %12
  store [2 x [2 x i64]] [[2 x i64] [i64 111, i64 8589934592], [2 x i64] [i64 111, i64 8589934592]], [2 x [2 x i64]]* %11
  store [3 x [2 x i64]] [[2 x i64] [i64 10, i64 20], [2 x i64] [i64 30, i64 40], [2 x i64] [i64 50, i64 60]], [3 x [2 x i64]]* %10
  store [2 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4]], [2 x [2 x i32]]* %9
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %8
  store [2 x i32] zeroinitializer, [2 x i32]* %7
  store [2 x i32] %0, [2 x i32]* %6
  store [2 x i32] %0, [2 x i32]* %6
  %28 = call [2 x i1] @testArray([2 x i32] %0, [2 x i1] %1, [2 x i8*] %2)
  store [2 x i1] %28, [2 x i1]* %5
  store [2 x i32] [i32 1, i32 2], [2 x i32]* %4
  ret [2 x i1] [i1 true, i1 false]
}

define i8* @testVoidPtr(i8* %0) {
  ret i8* %0
}

define i32 @main() {
  call void @testWhile()
  call void @fizzbuzz(i32 0, i32 100)
  ret i32 0
}

define void @testWhile() {
  %1 = alloca i32
  %2 = alloca i32
  store i32 0, i32* %2
  br label %3

3:                                                ; preds = %18, %0
  %4 = load i32, i32* %2
  %5 = icmp slt i32 %4, 10
  br i1 %5, label %6, label %21

6:                                                ; preds = %3
  store i32 0, i32* %1
  br label %7

7:                                                ; preds = %10, %6
  %8 = load i32, i32* %1
  %9 = icmp slt i32 %8, 10
  br i1 %9, label %10, label %18

10:                                               ; preds = %7
  %11 = load i32, i32* %2
  %12 = mul i32 10, %11
  %13 = load i32, i32* %1
  %14 = add i32 %12, %13
  %15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %14)
  %16 = load i32, i32* %1
  %17 = add i32 %16, 1
  store i32 %17, i32* %1
  br label %7

18:                                               ; preds = %7
  %19 = load i32, i32* %2
  %20 = add i32 %19, 1
  store i32 %20, i32* %2
  br label %3

21:                                               ; preds = %3
  %22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0))
  ret void
}

define void @fizzbuzz(i32 %0, i32 %1) {
  %3 = alloca i32
  %4 = alloca i32
  store i32 %0, i32* %4
  br label %5

5:                                                ; preds = %33, %2
  %6 = load i32, i32* %4
  %7 = icmp slt i32 %6, %1
  br i1 %7, label %8, label %36

8:                                                ; preds = %5
  %9 = load i32, i32* %4
  %10 = add i32 %9, 1
  store i32 %10, i32* %3
  %11 = load i32, i32* %3
  %12 = srem i32 %11, 15
  %13 = icmp eq i32 %12, 0
  br i1 %13, label %14, label %16

14:                                               ; preds = %8
  %15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @2, i32 0, i32 0))
  br label %33

16:                                               ; preds = %8
  %17 = load i32, i32* %3
  %18 = srem i32 %17, 5
  %19 = icmp eq i32 %18, 0
  br i1 %19, label %20, label %22

20:                                               ; preds = %16
  %21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @3, i32 0, i32 0))
  br label %32

22:                                               ; preds = %16
  %23 = load i32, i32* %3
  %24 = srem i32 %23, 3
  %25 = icmp eq i32 %24, 0
  br i1 %25, label %26, label %28

26:                                               ; preds = %22
  %27 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @4, i32 0, i32 0))
  br label %31

28:                                               ; preds = %22
  %29 = load i32, i32* %3
  %30 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @5, i32 0, i32 0), i32 %29)
  br label %31

31:                                               ; preds = %28, %26
  br label %32

32:                                               ; preds = %31, %20
  br label %33

33:                                               ; preds = %32, %14
  %34 = load i32, i32* %4
  %35 = add i32 %34, 1
  store i32 %35, i32* %4
  br label %5

36:                                               ; preds = %5
  %37 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @6, i32 0, i32 0))
  ret void
}
