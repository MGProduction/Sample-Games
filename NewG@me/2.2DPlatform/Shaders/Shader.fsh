//
//  Shader.fsh
//  P@cM@n
//
//  Created by Marco Giorgini on 21/04/13.
//  Copyright (c) 2013 Marco Giorgini. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
