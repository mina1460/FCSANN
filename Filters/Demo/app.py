import flask
import asyncio
import time
from pipeline import TextPipeline
from text_model import Result

textPipeline = TextPipeline()


async def async_get_data():
    await asyncio.sleep(10)
    return "Done!"


async def query_pipeline(data, filters, k):
    return textPipeline.query(data, filters, k)


app = flask.Flask(__name__)


@app.route("/")
def index():
    return flask.render_template("initial_screen.html")


@app.route("/text", methods=["GET", "POST"])
async def text():
    if flask.request.method == "GET":  # before entering query
        return flask.render_template("text_screen.html")
    else:  # after entering query
        print(flask.request.form)
        now = time.time()  # time before getting neighbors

        year_data = flask.request.form.get("year")
        if flask.request.form.get("yearAny") == "-1":
            print("-1")
            # year_data = "-1"

        filters = [
            textPipeline.get_filter("rating", flask.request.form.get("rating")),
            textPipeline.get_filter("vpurchase", flask.request.form.get("vpurchase")),
            textPipeline.get_filter("year", year_data),
            textPipeline.get_filter("category", flask.request.form.get("category")),
        ]
        data = await query_pipeline(
            flask.request.form.get("query_text"), filters, flask.request.form.get("k")
        )
        time_took = round(time.time() - now, 2)
        return flask.render_template(
            "text_screen.html", show_results=True, time=time_took, my_list=data
        )


@app.route("/image", methods=["GET", "POST"])
async def image():
    if flask.request.method == "GET":
        # return flask.render_template('image_screen.html')
        return flask.render_template("index.html")
    else:
        print(flask.request.form)
        now = time.time()  # time before getting neighbors
        filters = [
            textPipeline.get_filter("rating", flask.request.form.get("rating")),
            textPipeline.get_filter("vpurchase", flask.request.form.get("vpurchase")),
            textPipeline.get_filter("year", flask.request.form.get("year")),
            textPipeline.get_filter("category", flask.request.form.get("category")),
        ]
        # import pdb

        # pdb.set_trace()

        data = await query_pipeline(
            flask.request.form.get("query_text"), filters, flask.request.form.get("k")
        )
        time_took = round(time.time() - now, 2)

        filters_to_display = []
        for i, f in enumerate(filters):
            if len(f) > 1:
                filters_to_display.append("-1")
            else:
                if i == 3:
                    filters_to_display.append(textPipeline.get_category(f[0]))
                elif i == 2:
                    filters_to_display.append(textPipeline.get_year(f[0]))
                else:
                    filters_to_display.append(f[0])

        return flask.render_template(
            "results_page.html",
            query=flask.request.form.get("query_text"),
            f1=filters_to_display[0],
            f2=filters_to_display[1],
            f3=filters_to_display[2],
            f4=filters_to_display[3],
            show_results=True,
            time=time_took,
            my_list=data,
        )


@app.route("/css/main.css")
def css():
    return flask.render_template("css/main.css")


if __name__ == "__main__":
    app.run(port=1234)
    # app.run(debug=True)
