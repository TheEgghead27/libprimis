#ifndef MODEL_H_
#define MODEL_H_

enum
{
    MDL_MD5 = 0,
    MDL_OBJ,
    MDL_NumMDLTypes
};

/* model: the base class for an ingame model
 *
 * extended by animmodel (animated model) which is itself extended by skelmodel
 *
 * a model format loader (e.g. md5 or obj) extends model or one of its children
 * and assigns the data from the file format into the object (by setting its
 * fields and overriding its methods)
 *
 * model class hierarchy (all are objects except bottom gvars)
 *
 *  /-------\
 *  | model |
 *  \-------/
 *      |
 *      v
 *  /-----------\
 *  | animmodel |
 *  \-----------/
 *      |     \__________
 *      v                \
 *  /-----------\         v
 *  | skelmodel |       /-----------\
 *  \-----------/       | vertmodel |
 *    |                 \-----------/
 *    |     /-------------\       |
 *    |     | modelloader |       |
 *    |     \-------------/       |
 *    |            |              |
 *     \__________/ \_____________/  <-- multiple inheritance via template class
 *          |                 |
 *          v                 v
 *      /------------\      /------------\
 *      | skelloader |      | vertloader |
 *      \------------/      \------------/
 *          |                           |
 *          v     /---------------\     v
 *     /-----\    | modelcommands |    /-----\
 *     | md5 |    \---------------/    | obj |
 *     \-----/        |        |       \-----/
 *      |             v        v            |
 *      | /--------------\ /--------------\ |
 *      | | skelcommands | | vertcommands | |
 *      | \--------------/ \--------------/ |
 *      |   |                           |   |
 *      v   v                           v   v
 * /-------------------\    /-------------------\
 * | skelcommands<md5> |    | vertcommands<obj> |
 * | md5commands       |    | md5commands       |
 * \-------gvar--------/    \-------gvar--------/
 */
class model
{
    public:
        //for spin, orientation: x = yaw, y = pitch, z = roll
        vec spin,
            orientation;
        bool shadow, alphashadow, depthoffset;
        float scale;
        vec translate;
        std::unique_ptr<BIH> bih;
        vec bbextend;
        float eyeheight, collidexyradius, collideheight;
        std::string collidemodel;
        int collide, batch;

        virtual ~model()
        {
        }

        virtual void calcbb(vec &, vec &) const = 0;
        virtual void calctransform(matrix4x3 &) const = 0;
        virtual int intersect(int, int, int, const vec &, float, float, float, dynent *, modelattach *, float, const vec &, const vec &, float &, int) const = 0;
        virtual void render(int, int, int, const vec&, float, float, float , dynent *, modelattach * = nullptr, float = 1, const vec4<float>& = vec4<float>(1, 1, 1, 1)) const = 0;
        virtual bool load() = 0;
        virtual int type() const = 0;
        virtual bool setBIH() = 0;
        virtual bool skeletal() const = 0;
        virtual bool animated() const = 0;
        virtual bool pitched() const = 0;
        virtual bool alphatested() const = 0;

        virtual void setshader(Shader *) = 0;
        virtual void setspec(float) = 0;
        virtual void setgloss(int) = 0;
        virtual void setglow(float, float, float) = 0;
        virtual void setalphatest(float) = 0;
        virtual void setfullbright(float) = 0;
        virtual void setcullface(int) = 0;
        virtual void setcolor(const vec &) = 0;
        virtual void settransformation(const std::optional<vec>,
                                       const std::optional<vec>,
                                       const std::optional<vec>,
                                       const std::optional<float>) = 0;
        //returns the location and size of the model
        virtual vec4<float> locationsize() const = 0;
        virtual void genshadowmesh(std::vector<triangle> &, const matrix4x3 &) {}

        virtual void preloadBIH()
        {
            if(!bih)
            {
                setBIH();
            }
        }

        virtual void preloadshaders() = 0;
        virtual void preloadmeshes() = 0;
        virtual void cleanup() = 0;

        virtual void startrender() const = 0;
        virtual void endrender() const = 0;

        void boundbox(vec &center, vec &radius)
        {
            if(bbradius.x < 0)
            {
                calcbb(bbcenter, bbradius);
                bbradius.add(bbextend);
            }
            center = bbcenter;
            radius = bbradius;
        }

        float collisionbox(vec &center, vec &radius)
        {
            if(collideradius.x < 0)
            {
                boundbox(collidecenter, collideradius);
                if(collidexyradius)
                {
                    collidecenter.x = collidecenter.y = 0;
                    collideradius.x = collideradius.y = collidexyradius;
                }
                if(collideheight)
                {
                    collidecenter.z = collideradius.z = collideheight/2;
                }
                rejectradius = collideradius.magnitude();
            }
            center = collidecenter;
            radius = collideradius;
            return rejectradius;
        }

        float above()
        {
            vec center, radius;
            boundbox(center, radius);
            return center.z+radius.z;
        }

        const std::string &modelname() const
        {
            return name;
        }

    protected:
        model(std::string name) : spin(0, 0, 0),
                                  orientation(0, 0, 0),
                                  shadow(true),
                                  alphashadow(true),
                                  depthoffset(false),
                                  scale(1.0f),
                                  translate(0, 0, 0),
                                  bbextend(0, 0, 0),
                                  eyeheight(0.9f),
                                  collidexyradius(0),
                                  collideheight(0),
                                  collidemodel(""),
                                  collide(Collide_OrientedBoundingBox),
                                  batch(-1),
                                  name(name),
                                  bbcenter(0, 0, 0),
                                  bbradius(-1, -1, -1),
                                  collidecenter(0, 0, 0),
                                  collideradius(-1, -1, -1),
                                  rejectradius(-1)
                                  {}
    private:
        std::string name;
        vec bbcenter, bbradius, collidecenter, collideradius;
        float rejectradius;
};

#endif
