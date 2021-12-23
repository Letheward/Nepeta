# Manual

This manual is included in the distrubution so you can view it offline.   
You can find it in Rack by clicking Help - Open user folder, and go into `plugins/Nepeta/docs/`.

## BitMixer

Mixing *raw bits* of two incoming signals.  

Include some basic clipping and gain control, since the output can be **dangerously** large.

### Interface

- Big Knob (`Main Mix Mode`)  
    
    Bit mixing algorithm (mixing raw bits of A, B):  
    
    - 1: max of A, B
    - 2: A + B
    - 3: A - B
    - 4: A * B
    - 5: A / B
    - 6: A bit and B
    - 7: A bit or  B
    - 8: A bit xor B

- `sign` Knob  
    
    Sign mixing Mode (mixing A's sign bit with B's sign bit (denoted as A and B below). If not 0, paste the result on final bits from mixing)
    
    - 0: No Mix (bypass)
    - 1: Use A
    - 2: Use B
    - 3: A bit and B
    - 4: A bit or  B
    - 5: A bit xor B

- `bitshift` Knobs 1 and 2

    Shift A and B's bits before mixing (will not affect sign bits in sign mixing). Positive numbers means shift right, and negatives means shift left.

    You can open context menu, choose between `rotate` and `shift` mode.

- `main` Knob  

    Main Gain control, before clipping.

- `Data` Ports 1 and 2  

    Input signals, in above explanation, A is 1, B is 2.

- `Gain` Knobs beside `Data` Ports

    Pre bit-mixing Gain control for Inputs.

- `out` Port
    
    Main Output.

- Context Menu: `Bit Shift Mode`

    - `rotate`: bit rotate 
    - `shift`: bit shift (leave 0s)

- Context Menu: `Clipping Mode`

    - `hard`: hard clipping, clamp to -5V ~ 5V
    - `tanh`: simple soft distortion, clamp to -5V ~ 5V
    - `sqrt`: weird wavefolding, clamp to -5V ~ 5V
    - `CV`: hard clipping, clamp to 0V ~ 10V

### Tips

- Given audio signals, usually it will destroy everything (but may leave some character of the originals), thus this can be use to generate custom noises for percussions.   
    Finetuning can get you on some gnarly pitched FM-esque sounds.

- `rotate` is noiser than `shift`, `shift` will not have noticeable effect in some settings.

- Try to mix a VCO with a LFO, or a Kickdrum

- Spam the random shortcut!!! (Ctrl+R)   

---

## PolyXform

Multiply 2 polyphonic inputs like they are matrices. Support rectangle (and square) matrices from 1x1 to 4x4.

Each input also has a scale knob, since matrix multiplication can generate large values.

### Interface

- 2 pairs of `row` and `col`

    control matrix 1 and matrix 2's size

- `Matrix` Ports 1 and 2

    Polyphonic Inputs, will be fill into matrices by row.
    
    2 rows and 3 columns will give you a matrix like this (numbers in matrix are channel ids):
    ~~~
    1  2  3
    4  5  6
    ~~~
    
    However, the real matrices you get will be different if matrix 1's column count is not equal to matrix 2's row count (because how matrix multiplication is defined), in that case, this module will choose the lesser one of these two as the equal count. 
    
    For example, if you set matrix 1 be 3x2 and matrix 2 be 4x2, these are the actual matrices you get (numbers in matrices are channel ids):
    ~~~
    Matrix 1 (3x2)    Matrix 2 (cut to 2x2)    Result
    1  2              1  2                     *  *
    3  4              3  4                     *  *
    5  6                                       *  *
    ~~~


- `Scale` Knobs beside `Matrix` Ports

    Scale every value in each matrix by this before multiplication.

- `out` Port

    Polyphonic output, output the matrix from multiplication by row.   
    For example (numbers in matrix are values):
    ~~~
    Matrix 
    1.0  9.0  2.0
    0.0  4.0  8.0

    Output Channels:
    1    2    3    4    5    6
    1.0  9.0  2.0  0.0  4.0  8.0
    ~~~

    Matrix size / channel count is determined by sizes of two matrix inputs, specifically, row 1 by column 2.

- Matrix light display

    Display all values in the result matrix.

### Tips

- This module works like a normal VCA with mono inputs (envelope into matrix 1 and audio into matrix 2).  
    However, when plug in polyphonic audio and envelopes, things get strange: it won't play the note you are playing, but some other note you play before or nothing, based on how you set up sizes of matrices and polyphonic input (for example, rotate vs reset, etc.).

- Feed polyphonic oscillators (especially with different frequencies) into both matrix can give you dense sound cluster. You can tune the result by messing with input matrix sizes.

- Since N by 1 matrices (vectors) is also valid, this can "integrate" with the other module **Rotor** (see below). You will need some polyphonic split/merge modules (included in Fundamental).

---

## Rotor

Mixing 3 inputs like rotating a 3D vector using rotors from Geometric Algebra.

Include 2 rotors, one from 4 Knobs, one from 4 Inputs. Rotate input vector by the product of 2 rotors. 

### Interface

- 4 Inputs and 4 Knobs
    
    A rotor is (s, yz, zx, xy)
    - `s`:  scalar (from dot product)
    - `yz`: yz bivector component (x axis rotation)
    - `zx`: zx bivector component (y axis rotation)
    - `xy`: xy bivector component (z axis rotation)
    
    Thus give us 2 rotors. (note: these are not "half angle" rotors)  
    
- 3 Inputs and 3 Outputs

    In vector and Out vector, denoted as usual: (x, y, z).  
    In and Out in each row have same axis.

- Context Menu: `Rotor Input Mode`
    
    In the 4 rotor inputs:
    - Bipolar: map -5V ~ 5V to -1 ~ 1
    - Unipolar: map 0V ~ 10V to -1 ~ 1

### Tips

- Set `s` knob to 1, and try turning other three knobs individually.

- Rotor Inputs needs to have at least 2 connected to have effect.

- Try plug audio signal to the rotor inputs.

- Treat this module as a strange 3 channel panner if you don't know the Math.  
    You can watch these 2 excellent videos to learn about Geometric Algebra:
    - [A Swift Introduction to Geometric Algebra](https://www.youtube.com/watch?v=60z_hpEAtD8)
    - [Let's remove Quaternions from every 3D Engine: Intro to Rotors from Geometric Algebra](https://www.youtube.com/watch?v=Idlv83CxP-8)

---

