# Debug Draw Points

The **Debug Draw Points** node takes a list of points and draws them in the editor viewport. This does not work in a built game.

![Debug Draw Points](media/debug-draw-points.png)

## Properties
| Property | Description |
|--------|--------|
| **Point Color** | The color of the points to be drawn. |

![Debug Draw Points Node Properties](media/debug-draw-points-properties.png)

## Example
### Graph:
The following graph finds all box colliders inside the volume, samples points on them, and then draws debug points at those points.
![example graph](media\debug-draw-points-example-graph.png)

### Output:

![A cube with PLCT points projected onto it.](media\debug-draw-points-example-output.png)