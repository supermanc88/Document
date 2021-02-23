# Event Order 事件触发顺序

 在考虑GameMaker Studio 2中的事件时，应注意，不能明确说明每个事件将要发生的确切顺序，仅因为它取决于GameMaker Studio 2的内部工作原理，并且可能会随着软件的发展发生变化。但是，某些事件将始终以相同的顺序运行。

总是以相同方式发生的**第一组事件是首次进入房间时发生的事件**。不同事件将触发的顺序是：

- **Object Variables / Instance Variables are initialised** 对象变量/实例变量初始化（这些变量可以在对象编辑器或房间编辑器的IDE中定义）。如果您有一个带有变量的对象，则将在其他所有对象之前创建它们，然后，将在之后定义任何带变量的实例，因此您可以在房间编辑器中用实例特定的对象覆盖对象变量。
- **CreateEvent**  创建事件后跟每个实例的实例创建代码-因此，在创建每个实例时，它将首先运行其创建事件，然后运行其实例创建代码，然后再继续下一个要创建的实例
- **Game Start Event** 游戏开始事件-从房间编辑器放置在房间的所有实例中，在游戏的第一个房间中触发一次该事件（请注意，调用game_restart（）将再次触发此事件。
- **Room Creation Code** 房间创建代码-这是在“房间编辑器”中编写的一次性代码，用于首次输入房间时
- **Room Start Event of all instances** 所有实例的“房间开始事件”-事件的“other”类别之一，将针对所有实例（持久性或其他）触发

还要注意的是，您还可以通过在实例编辑器属性窗口的列表中上移或下移来设置特定实例在房间编辑器中的创建顺序。

除了那些特定的事件，无论属于三种step事件和不同的draw事件，唯一已知的顺序将始终以相同的方式发生。这些将始终保持一致，因此，如果您的代码在游戏的每个步骤中都依赖于特定的时间安排，则应使用：

- **Begin Step Event**
- **Step Event** 请注意，步骤事件是在实例放到新位置之前执行的
- **End Step Event**

所有用于绘制的事件也总是按以下相同顺序处理（“窗口调整大小”事件除外，该事件的触发方式不同）：

- **Pre Draw Event**
- **Draw Begin Event**
- **Draw Event**
- **Draw End Event**
- **Post Draw Event**
- **Draw GUI Begin Event**
- **Draw GUI Event**
- **Draw GUI End Event**

