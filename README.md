# Powder
This prototype was made in roughly a week as a way to solidify some of the C knowledge I had learnt in classes. The gameplay idea was to have limited resources that you could feed to the block in the center. As you fed the block you would recieve more resources back which you could use to drop other types of pixels. The eventual goal was to have an automated sandbox idle game where you could create complex systems to combine elements for you feeding the block for you to create even more systems and interactions. 

<h2>Particle System</h2>
The particle system was largely based off a GDC talk for the game <a href="https://www.youtube.com/watch?v=prXuyMCgbTc">Noita</a>. The simulation world is a 2D array where each cell contains a Pixel type with the data associated with that cell. Upon spawning a pixel, it checks the cells immediately down, down-left, then down-right to see if and where it should fall. Depending on the type of pixel it will interact and move in a different way. Pixel types have their own values such as density, type, location, flammability, etc.

<h2>Heat System / Debug Mode</h2>
In planning interactions between pixel types, some are easier to visualize than others. Powder falls through water where as oil floats on top of water is easy enough to see, but for the fire and wind I wanted empty cells to still have effects placed on any pixel that enters them. I was quite happy with this debug system that you could toggle to see a visual representation of heat values, or directions of fan velocities.

<h2>Interactions</h2>
<p>
Current supported interactions
  -Powder burns quickly
  -Water puts out fire, melts snow slowly
  -Fire burns powder/oil and increases heat in surrounding cells, melts snow quickly
  -Oil floats on water, burns slowly
  -Snow falls in a floaty pattern, when it melts from fire or water it turns into water
  -Fan (WIP) static block that pushes nearby pixels
</p>
