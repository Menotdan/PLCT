# Filter By Normal

The **Filter By Normal** node takes a list of points and creates a new list of points where the "Normal" property is between the specified minimum and maximum values. Each component of the normal vector is checked separately. This node is useful to limit the slope that a point can be at, since, for example, trees don't often grow at 90 degree angles.

![Filter By Normal Node](media/filter-by-normal.png)

## Properties
| Property | Description |
|--------|--------|
| **Min Value** | The lowest value of the "Normal" property for this point to be included in the output. |
| **Max Value** | The highest value of the "Normal" property for this point to be included in the output. |

![Filter By Normal Node Properties](media/filter-by-normal-properties.png)

## Example
The following graph samples points on a terrain, and then filters the points by their normal vectors, making sure the normal vector is close to straight up (minimum Y component of 0.8) to prevent trees from spawning on cliffs.



### Graph:
![A graph using the filter by normal node.](media\filter-by-normal-example-graph.png)

### Output:

![Trees not spawning on a cliff, but spawning on the ground.](media\filter-by-normal-example-output.png)