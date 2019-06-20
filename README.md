# KMLpathMerge

This is a command line utility to merge multiple paths (or poly-lines) in a KML file, producing a new KML file with a single path.
KML files can be produced by Google Earth or many other GIS systems.
They may also be created by unzipping a KMZ.
The path segments must be arranged in proper sequence.
This utility will decide which way to flip each segment so that they fit together best.
Or, put another way, it does what you cannot do with Google Earth.
